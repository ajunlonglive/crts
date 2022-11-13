#include "posix.h"

#include <math.h>

#include "client/client.h"
#include "client/ui/gl/colors.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/chunks.h"
#include "client/ui/gl/shader.h"
#include "client/ui/gl/shader_multi_obj.h"
#include "client/ui/gl/ui.h"
#include "shared/math/geom.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"
#include "tracy.h"

#define MAX_RENDERED_CHUNKS 2048

static struct {
	uint32_t count;
	GLsizei draw_counts[MAX_RENDERED_CHUNKS];
	const GLvoid *draw_indices[MAX_RENDERED_CHUNKS];
	GLint draw_baseverts[MAX_RENDERED_CHUNKS];
} s_chunk = { 0 };

enum chunk_uniform {
	cu_colors = UNIFORM_START_RP_FINAL,
};

struct shader chunk_shader = { 0 };
struct shader chunk_bottom_shader = { 0 };

enum feature_type {
	feat_tree,
	feat_block,
	feat_dodec,
	feat_count
};

static struct model_spec feature_model[feat_count][detail_levels] = {
	[feat_tree]  = { { "pyramid.obj", 1.0 } },
	[feat_block] = { { "cube.obj", 1.0 }, },
	[feat_dodec] = { { "dodecahedron.obj", 1.0 }, },
};

struct shader_multi_obj feat_shader;

static uint32_t chunk_indices_reversed_winding[CHUNK_INDICES_LEN] = { 0 };

bool
render_world_setup_chunks(struct hdarr *chunk_meshes)
{
	uint32_t i;
	for (i = 0; i < CHUNK_INDICES_LEN; i += 3) {
		chunk_indices_reversed_winding[i + 0] = chunk_indices[i + 2];
		chunk_indices_reversed_winding[i + 1] = chunk_indices[i + 1];
		chunk_indices_reversed_winding[i + 2] = chunk_indices[i + 0];
	}

	struct shader_spec chunk_spec = {
		.src = {
			[rp_final] = {
				{ "chunks.vert", GL_VERTEX_SHADER },
				{ "world.frag", GL_FRAGMENT_SHADER },
			},
			[rp_depth] = {
				{ "chunks_depth.vert", GL_VERTEX_SHADER },
				{ "empty.frag", GL_FRAGMENT_SHADER },
			},
		},
		.uniform = { [rp_final] = { { cu_colors, "colors" } } },
		.attribute = {
			{ { 3, GL_FLOAT, bt_vbo, true }, { 3, GL_FLOAT, bt_vbo },
			  { 1, GL_FLOAT, bt_vbo } }
		},
		.static_data = {
			{ chunk_indices, sizeof(uint32_t) * CHUNK_INDICES_LEN, bt_ebo },
		},
		.interleaved = true
	};

	if (!shader_create(&chunk_spec, &chunk_shader)) {
		return false;
	}

	struct shader_spec chunk_bottom_spec = {
		.src = {
			[rp_final] = {
				{ "chunk_bottoms.vert", GL_VERTEX_SHADER },
				{ "basic.frag", GL_FRAGMENT_SHADER },
			},
		},
		.attribute = {
			{ { 3, GL_FLOAT, bt_vbo, true }, { 3, GL_FLOAT, bt_vbo },
			  { 1, GL_FLOAT, bt_vbo } }
		},
		.static_data = {
			{ chunk_indices_reversed_winding, sizeof(uint32_t) * CHUNK_INDICES_LEN, bt_ebo },
		},
		.uniform_blacklist = {
			[rp_final] = 0xffff & ~(1 << duf_viewproj | 1 << duf_clip_plane),
		},
		.interleaved = true
	};

	if (!shader_create(&chunk_bottom_spec, &chunk_bottom_shader)) {
		return false;
	}

	/* create chunk mesh hdarr */
	hdarr_init(chunk_meshes, 2048, sizeof(struct point), sizeof(chunk_mesh), NULL);

	glUseProgram(chunk_shader.id[rp_final]);
	glUniform4fv(chunk_shader.uniform[rp_final][cu_colors], tile_count, (float *)colors.tile);

	if (!shader_create_multi_obj(feature_model, feat_count, &feat_shader)) {
		return false;
	}

	return true;
}

static void
add_feature(enum tile t, struct chunk_info *ci)
{
	enum feature_type feat_type;
	obj_data feat = { 0 };
	float scale = 1.0;

	/* add features */
	switch (t) {
	case tile_old_tree:
	case tile_tree:
		feat_type = feat_tree;
		break;
	default:
		return;
	}

	feat[0] = ci->pos[0] + 0.5;
	feat[1] = ci->pos[1] + 0.5;
	feat[2] = ci->pos[2] + 0.5;
	feat[3] = scale;
	feat[4] = colors.tile[t][0];
	feat[5] = colors.tile[t][1];
	feat[6] = colors.tile[t][2];
	feat[7] = 1.0f;

	smo_push(&feat_shader, feat_type, feat);
}

static void
setup_chunks(struct client *cli, struct chunks *cnks, struct gl_ui_ctx *ctx, struct hdarr *cms)
{
	struct chunk *ck, *rck, *bck, *cck;
	uint32_t x, y;
	enum tile t;
	uint16_t i;
	float h;
	bool inside_chunk;

	s_chunk.count = 0;

	struct rect containing_rect;
	containing_axis_aligned_rect(&cli->ref.rect, &containing_rect);

	chunk_mesh mesh = { 0 }, *draw_mesh;
	struct point adjp, onp, np = onp = nearest_chunk(&(struct point) {
		containing_rect.p[0].x, containing_rect.p[0].y
	});

	for (; np.x < containing_rect.p[0].x + containing_rect.w; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < containing_rect.p[0].y + containing_rect.h; np.y += CHUNK_SIZE) {
			if ((draw_mesh = hdarr_get(cms, &np))) {
				goto draw_chunk_mesh;
			}

			if (!(ck = hdarr_get(&cnks->hd, &np))) {
				continue;
			}

			adjp = np;
			adjp.x += CHUNK_SIZE;
			rck = hdarr_get(&cnks->hd, &adjp);

			adjp.y += CHUNK_SIZE;
			cck = hdarr_get(&cnks->hd, &adjp);

			adjp.x -= CHUNK_SIZE;
			bck = hdarr_get(&cnks->hd, &adjp);

			for (y = 0; y < MESH_DIM; ++y) {
				for (x = 0; x < MESH_DIM; ++x) {
					t = h = 0;
					inside_chunk = true;

					if (x >= CHUNK_SIZE && y >= CHUNK_SIZE) {
						inside_chunk = false;
						if (cck) {
							t = cck->tiles[0][0];
							h = cck->heights[0][0];
						}
					} else if (x >= CHUNK_SIZE) {
						inside_chunk = false;
						if (rck) {
							t = rck->tiles[0][y];
							h = rck->heights[0][y];
						}
					} else if (y >= CHUNK_SIZE) {
						inside_chunk = false;
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
					 *
					 * ^
					 * You need to use VertexAttribIPointer
					 * I think
					 * */
					mesh[i].type = (float)t;

					/* add feature */
					if (inside_chunk) {
						add_feature(t, &mesh[i]);
					}
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
			glBindBuffer(GL_ARRAY_BUFFER, chunk_shader.buffer[bt_vbo]);
			glBufferSubData(GL_ARRAY_BUFFER,
				s_chunk.count * sizeof(chunk_mesh),
				sizeof(chunk_mesh),
				*draw_mesh);

			glBindBuffer(GL_ARRAY_BUFFER, chunk_bottom_shader.buffer[bt_vbo]);
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

void
render_chunks_setup_frame(struct client *cli, struct gl_ui_ctx *ctx, struct hdarr *cms)
{
	TracyCZoneAutoS;
	ctx->reset_chunks = ctx->ref_changed || cli->changed.chunks;

	if (ctx->reset_chunks) {
		glBindBuffer(GL_ARRAY_BUFFER, chunk_shader.buffer[bt_vbo]);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
			NULL, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, chunk_bottom_shader.buffer[bt_vbo]);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
			NULL, GL_DYNAMIC_DRAW);

		smo_clear(&feat_shader);

		hdarr_clear(cms);

		setup_chunks(cli, &cli->world->chunks, ctx, cms);
	}

	if (ctx->reset_chunks || cam.changed) {
		smo_upload(&feat_shader);
	}
	TracyCZoneAutoE;
}

void
render_chunks(struct client *cli, struct gl_ui_ctx *ctx, struct hdarr *cms, bool render_bottoms)
{
	TracyCZoneAutoS;

	if (ctx->pass == rp_final) {
		glUseProgram(chunk_bottom_shader.id[ctx->pass]);
		glBindVertexArray(chunk_bottom_shader.vao[ctx->pass][0]);
		shader_check_def_uni(&chunk_bottom_shader, ctx);
		glMultiDrawElementsBaseVertex(
			GL_TRIANGLES,
			s_chunk.draw_counts,
			GL_UNSIGNED_INT,
			s_chunk.draw_indices,
			s_chunk.count,
			s_chunk.draw_baseverts);
	}

	glUseProgram(chunk_shader.id[ctx->pass]);
	glBindVertexArray(chunk_shader.vao[ctx->pass][0]);
	shader_check_def_uni(&chunk_shader, ctx);
	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		s_chunk.draw_counts,
		GL_UNSIGNED_INT,
		s_chunk.draw_indices,
		s_chunk.count,
		s_chunk.draw_baseverts);

#ifndef NDEBUG
	ctx->prof.chunk_count = s_chunk.count;
#endif

	smo_draw(&feat_shader, ctx);

	TracyCZoneAutoE;
}
