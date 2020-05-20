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

#define MESH_DIM (CHUNK_SIZE + 1)
typedef float chunk_mesh[MESH_DIM * MESH_DIM][3][3];

static struct {
	uint32_t id;
	uint32_t vao, vbo;
	uint32_t view, proj, view_pos, positions, types;
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
	struct hdarr *hd;
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

	s_chunk.hd = hdarr_init(2048, sizeof(struct point), sizeof(chunk_mesh), NULL);

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
	hdarr_destroy(s_chunk.hd);
}

void
update_world_viewport(mat4 mproj)
{
	glUseProgram(s_ent.id);
	glUniformMatrix4fv(s_ent.proj, 1, GL_TRUE, (float *)mproj);

	glUseProgram(s_chunk.id);
	glUniformMatrix4fv(s_chunk.proj, 1, GL_TRUE, (float *)mproj);
}

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
	float h;

	chunk_mesh mesh = { 0 }, *draw_mesh;

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if ((draw_mesh = hdarr_get(s_chunk.hd, &sp))) {
				goto draw_chunk_mesh;
			} else if (!(ck = hdarr_get(cnks->hd, &sp))) {
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
					t = h = 0;
					if (x >= CHUNK_SIZE && y >= CHUNK_SIZE) {
						if (cck) {
							t = cck->tiles[0][0];
							h = cck->heights[0][0];
						}
					} else if (x >= CHUNK_SIZE) {
						if (rck) {
							t = rck->tiles[0][y];
							h = rck->heights[0][y];
						}
					} else if (y >= CHUNK_SIZE) {
						if (bck) {
							t = bck->tiles[x][0];
							h = bck->heights[x][0];
						}
					} else {
						t = ck->tiles[x][y];
						h = ck->heights[x][y];
					}

					i = y * MESH_DIM + x;

					mesh[i][0][0] = ck->pos.x + x - 0.5;
					mesh[i][0][1] = h;
					mesh[i][0][2] = ck->pos.y + y - 0.5;

					mesh[i][1][0] = colors.tile[t][0];
					mesh[i][1][1] = colors.tile[t][1];
					mesh[i][1][2] = colors.tile[t][2];

					mesh[i][2][0] = 0;
					mesh[i][2][1] = 0;
					mesh[i][2][2] = 0;
				}
			}

			vec4 a = { 0 }, b = { 0 }, c = { 0 };

			for (x = 0; x < CHUNK_INDICES_LEN; x += 6) {
				memcpy(a, mesh[chunk_indices[x + 0]][0], sizeof(float) * 3);
				memcpy(b, mesh[chunk_indices[x + 1]][0], sizeof(float) * 3);
				memcpy(c, mesh[chunk_indices[x + 2]][0], sizeof(float) * 3);

				vec4_sub(b, a);
				vec4_sub(c, a);
				vec4_cross(b, c);

				memcpy(mesh[chunk_indices[x + 2]][2], b, sizeof(float) * 3);
			}

			hdarr_set(s_chunk.hd, &ck->pos, mesh);
			draw_mesh = &mesh;

draw_chunk_mesh:
			glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * MESH_DIM * MESH_DIM * 3 * 3,
				*draw_mesh, GL_STREAM_DRAW);

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

	float positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };

	for (i = 0, j = 0; i < len; ++i, ++j) {
		if (i >= 256) {
			glUniform1uiv(s_ent.types, 256, types);
			glUniform3fv(s_ent.positions, 256, positions);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256);
			j = 0;
		}

		struct point p = nearest_chunk(&emem[i].pos);
		struct chunk *ck = hdarr_get(cnks, &p);

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;
		if (ck) {
			p = point_sub(&emem[i].pos, &ck->pos);
			positions[(j * 3) + 2] = ck->heights[p.x][p.y];
		}

		types[j] = emem[i].type;
	}

	glUniform1uiv(s_ent.types, 256, types);
	glUniform3fv(s_ent.positions, 256, positions);
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

	if (hf->sim->changed.chunks) {
		hdarr_clear(s_chunk.hd);
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
