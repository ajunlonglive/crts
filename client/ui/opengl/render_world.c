#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render_world.h"
#include "client/ui/opengl/solids.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/math/linalg.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

static struct {
	uint32_t id;
	uint32_t vao, vbo;
	uint32_t view, proj, view_pos, positions, types;
	struct hash *h;
} s_ent = { 0 };

static bool
render_world_setup_ents(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/ents.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_ent.id)) {
		return false;
	}

	s_ent.view      = glGetUniformLocation(s_ent.id, "view");
	s_ent.proj      = glGetUniformLocation(s_ent.id, "proj");
	s_ent.positions = glGetUniformLocation(s_ent.id, "positions");
	s_ent.types     = glGetUniformLocation(s_ent.id, "types");
	s_ent.view_pos  = glGetUniformLocation(s_ent.id, "view_pos");

	s_ent.h = hash_init(2048, 1, sizeof(struct point));

	glGenVertexArrays(1, &s_ent.vao);
	glGenBuffers(1, &s_ent.vbo);

	glBindVertexArray(s_ent.vao);
	glBindBuffer(GL_ARRAY_BUFFER, s_ent.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * solid_cube.len,
		solid_cube.verts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return true;
}

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos;
} s_chunk = { 0 };

bool
render_world_setup_chunks(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/chunks.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_chunk.id)) {
		return false;
	}
	s_chunk.view      = glGetUniformLocation(s_chunk.id, "view");
	s_chunk.proj      = glGetUniformLocation(s_chunk.id, "proj");
	s_chunk.view_pos  = glGetUniformLocation(s_chunk.id, "view_pos");

	glGenVertexArrays(1, &s_chunk.vao);
	glGenBuffers(1, &s_chunk.vbo);
	glGenBuffers(1, &s_chunk.ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint16_t) * CHUNK_INDICES_LEN, chunk_indices,
		GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	glBindVertexArray(s_chunk.vao);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	return true;
}

bool
render_world_setup(char *graphics_path)
{
	return render_world_setup_ents()
	       && render_world_setup_chunks()
	       && color_cfg(graphics_path);
}

void
render_world_teardown(void)
{
	hash_destroy(s_ent.h);
}

void
update_world_viewport(mat4 mproj)
{
	glUseProgram(s_ent.id);
	glUniformMatrix4fv(s_ent.proj, 1, GL_TRUE, (float *)mproj);

	glUseProgram(s_chunk.id);
	glUniformMatrix4fv(s_chunk.proj, 1, GL_TRUE, (float *)mproj);
}

const float heights[] = {
	4.8,  //tile_deep_water,
	4.8,  //tile_water,
	5,  //tile_wetland,
	5.1,  //tile_plain,
	6.4,  //tile_forest,
	9.3,  //tile_mountain,
	11.3,  //tile_peak,
	5.1,  //tile_dirt,
	5.5, //tile_forest_young,
	6.1,  //tile_forest_old,
	5.5,  //tile_wetland_forest_young,
	6.1,  //tile_wetland_forest,
	6.1,  //tile_wetland_forest_old,
	4.8,   //tile_coral,

	6,  //tile_wood,
	6,  //tile_stone,
	5.2,  //tile_wood_floor,
	5.2,  //tile_rock_floor,
	8,  //tile_shrine,
	5,  //tile_farmland_empty,
	5.4,  //tile_farmland_done,
	5,  //tile_burning,
	5.1  //tile_burnt,
};

#define MESH_DIM (CHUNK_SIZE + 1)
static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *ck, *rck, *bck, *cck;
	struct point sp = nearest_chunk(&ctx->ref.pos), adjp;
	int spy = sp.y,
	    endx = ctx->ref.pos.x + ctx->ref.width,
	    endy = ctx->ref.pos.y + ctx->ref.height,
	    x, y;
	enum tile t;
	uint16_t i;

	float mesh[MESH_DIM * MESH_DIM][3][3] = { 0 };

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if (!(ck = hdarr_get(cnks->hd, &sp))) {
				continue;
			}

			adjp = sp;
			adjp.x += CHUNK_SIZE;
			rck = hdarr_get(cnks->hd, &adjp);

			adjp.y += CHUNK_SIZE;
			cck = hdarr_get(cnks->hd, &adjp);

			adjp.x -= CHUNK_SIZE;
			bck = hdarr_get(cnks->hd, &adjp);

			for (y = 0; y < MESH_DIM; ++y) {
				for (x = 0; x < MESH_DIM; ++x) {
					if (x >= CHUNK_SIZE && y >= CHUNK_SIZE) {
						t = cck ? cck->tiles[0][0] : 0;
					} else if (x >= CHUNK_SIZE) {
						t = rck ? rck->tiles[0][y] : 0;
					} else if (y >= CHUNK_SIZE && bck) {
						t = bck ? bck->tiles[x][0] : 0;
					} else {
						t = ck->tiles[x][y];
					}

					i = y * MESH_DIM + x;

					mesh[i][0][0] = ck->pos.x + x - 0.5;
					mesh[i][0][1] = heights[t];
					mesh[i][0][2] = ck->pos.y + y - 0.5;
					//L("%d, %d -> %d, %f, %f", x, y, y * MESH_DIM + x, ck->pos.x + x + 0.5, ck->pos.y + y + 0.5);

					mesh[i][1][0] = colors.tile[t][0];
					mesh[i][1][1] = colors.tile[t][1];
					mesh[i][1][2] = colors.tile[t][2];

					mesh[i][2][0] = 0;
					mesh[i][2][1] = 0;
					mesh[i][2][2] = 0;
				}
			}

			vec4 a = { 0 }, b = { 0 }, c = { 0 };

			for (x = 3; x < CHUNK_INDICES_LEN; x += 6) {
				memcpy(a, mesh[chunk_indices[x + 0]][0], sizeof(float) * 3);
				memcpy(b, mesh[chunk_indices[x + 1]][0], sizeof(float) * 3);
				memcpy(c, mesh[chunk_indices[x + 2]][0], sizeof(float) * 3);

				vec4_sub(b, a);
				vec4_sub(c, a);
				vec4_cross(b, c);

				memcpy(mesh[chunk_indices[x + 2]][2], b, sizeof(float) * 3);
			}

			glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * MESH_DIM * MESH_DIM * 3 * 3,
				mesh, GL_STREAM_DRAW);

			glDrawElements(GL_TRIANGLES, CHUNK_INDICES_LEN,
				GL_UNSIGNED_SHORT, (void *)0);

			//glDrawArrays(GL_POINTS, 0, MESH_DIM * MESH_DIM);
		}
	}
}

static void
render_ents(struct hdarr *ents, struct hdarr *cnks, struct opengl_ui_ctx *ctx)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(ents));
	size_t i, j, len = hdarr_len(ents);

	hash_clear(s_ent.h);

	int32_t positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };
	const size_t *st;

	for (i = 0, j = 0; i < len; ++i, ++j) {
		if (i >= 256) {
			glUniform1uiv(s_ent.types, 256, types);
			glUniform3iv(s_ent.positions, 256, positions);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256);
			j = 0;
		}

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;

		if ((st = hash_get(s_ent.h, &emem[i].pos))) {
			positions[(j * 3) + 2] = *st + 1;
			hash_set(s_ent.h, &emem[i].pos, *st + 1);
		} else {
			positions[(j * 3) + 2] = 0;
			hash_set(s_ent.h, &emem[i].pos, 0);
		}

		types[j] = emem[i].type;
	}

	glUniform1uiv(s_ent.types, 256, types);
	glUniform3iv(s_ent.positions, 256, positions);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, len % 256);
}

void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	mat4 mview;
	float w, h;

	if (cam.changed || ctx->resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		ctx->ref.pos = hf->view;

		if (cam.unlocked) {
			w = 128;
			h = 128;
		} else {
			w = cam.pos[1] * (float)ctx->width / (float)ctx->height / 2;
			h = cam.pos[1] * tanf(FOV / 2) * 2;
			cam.pos[0] = ctx->ref.pos.x + w * 0.5;
			cam.pos[2] = ctx->ref.pos.y + h * 0.5;
		}

		ctx->ref.width = w;
		ctx->ref.height = h;

		cam.tgt[0] = cos(cam.yaw) * cos(cam.pitch);
		cam.tgt[1] = sin(cam.pitch);
		cam.tgt[2] = sin(cam.yaw) * cos(cam.pitch);

		gen_look_at(&cam, mview);
		cam.changed = true;
	}

	/* chunks */

	glUseProgram(s_chunk.id);
	glBindVertexArray(s_chunk.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_chunk.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_chunk.view_pos, 1, cam.pos);
	}

	render_chunks(hf->sim->w->chunks, ctx);

	/* ents */

	glUseProgram(s_ent.id);
	glBindVertexArray(s_ent.vao);

	if (cam.changed) {
		glUniformMatrix4fv(s_ent.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_ent.view_pos, 1, cam.pos);
	}

	render_ents(hf->sim->w->ents, hf->sim->w->chunks->hd, ctx);

	cam.changed = false;
}
