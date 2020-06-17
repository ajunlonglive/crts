#include "posix.h"

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render/chunks.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/sim/chunk.h"

#define MAX_RENDERED_CHUNKS 512

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos, colors;
	uint32_t count;
	GLsizei draw_counts[MAX_RENDERED_CHUNKS];
	const GLvoid *draw_indices[MAX_RENDERED_CHUNKS];
	GLint draw_baseverts[MAX_RENDERED_CHUNKS];
} s_chunk = { 0 };

bool
render_world_setup_chunks(struct hdarr **chunk_meshes)
{
	struct shader_src src[] = {
		{ "chunks.vert", GL_VERTEX_SHADER },
		{ "world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_chunk.id)) {
		return false;
	}

	glUseProgram(s_chunk.id);

	s_chunk.view      = glGetUniformLocation(s_chunk.id, "view");
	s_chunk.proj      = glGetUniformLocation(s_chunk.id, "proj");
	s_chunk.view_pos  = glGetUniformLocation(s_chunk.id, "view_pos");
	s_chunk.colors    = glGetUniformLocation(s_chunk.id, "colors");

	glGenVertexArrays(1, &s_chunk.vao);
	glGenBuffers(1, &s_chunk.vbo);
	glGenBuffers(1, &s_chunk.ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint16_t) * CHUNK_INDICES_LEN, chunk_indices,
		GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	glBindVertexArray(s_chunk.vao);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_info),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_info),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// type attribute
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_info),
		(void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// create chunk mesh hdarr
	*chunk_meshes = hdarr_init(2048, sizeof(struct point), sizeof(chunk_mesh), NULL);

	glUniform4fv(s_chunk.colors, tile_count, (float *)colors.tile);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
		NULL, GL_DYNAMIC_DRAW);

	return true;
}

static void
setup_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx, struct hdarr *cms)
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
	s_chunk.count = 0;

	chunk_mesh mesh = { 0 }, *draw_mesh;

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if ((draw_mesh = hdarr_get(cms, &sp))) {
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

					mesh[i].pos[0] = ck->pos.x + x - 0.5;
					mesh[i].pos[1] = h;
					mesh[i].pos[2] = ck->pos.y + y - 0.5;

					mesh[i].norm[0] = 0;
					mesh[i].norm[1] = 0;
					mesh[i].norm[2] = 0;

					/* TODO: figure out how to do this
					 * without this dumb cast.  Basically
					 * when I try to use a uint the value
					 * gets mangled on its way to the gpu
					 * */
					mesh[i].type = (float)t;
				}
			}

			for (x = 0; x < CHUNK_INDICES_LEN; x += 6) {
				calc_normal(mesh[chunk_indices[x + 0]].pos,
					mesh[chunk_indices[x + 1]].pos,
					mesh[chunk_indices[x + 2]].pos,
					mesh[chunk_indices[x + 2]].norm);
			}

			hdarr_set(cms, &ck->pos, mesh);
			draw_mesh = &mesh;

draw_chunk_mesh:
			glBufferSubData(GL_ARRAY_BUFFER,
				s_chunk.count * sizeof(chunk_mesh),
				sizeof(chunk_mesh),
				*draw_mesh);

			s_chunk.draw_counts[s_chunk.count] = CHUNK_INDICES_LEN;
			((GLvoid **)s_chunk.draw_indices)[s_chunk.count] = (void *)0;
			s_chunk.draw_baseverts[s_chunk.count] = s_chunk.count
								* MESH_DIM * MESH_DIM;

			if (++s_chunk.count >= MAX_RENDERED_CHUNKS) {
				return;
			}
		}
	}
}

bool
render_chunks(struct hiface *hf, struct opengl_ui_ctx *ctx, struct hdarr *cms,
	mat4 mview, bool ref_changed)
{
	glUseProgram(s_chunk.id);
	glBindVertexArray(s_chunk.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_chunk.proj, 1, GL_TRUE, (float *)ctx->mproj);
		glUniformMatrix4fv(s_chunk.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_chunk.view_pos, 1, cam.pos);
	}

	bool reset_chunks = false;
	if (ref_changed || hf->sim->changed.chunks) {
		/* orphan previous buffer */
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
			NULL, GL_DYNAMIC_DRAW);

		hdarr_clear(cms);
		setup_chunks(hf->sim->w->chunks, ctx, cms);

		reset_chunks = true;
	}

	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		s_chunk.draw_counts,
		GL_UNSIGNED_SHORT,
		s_chunk.draw_indices,
		s_chunk.count,
		s_chunk.draw_baseverts);

	return reset_chunks;
}
