#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/render/chunks.h"
#include "client/ui/opengl/shader.h"
#include "client/ui/opengl/ui.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

#define MAX_RENDERED_CHUNKS 512

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

typedef float feature_instance[7];

enum feature_type {
	feat_tree,
	feat_tree_small,
	feat_block,
	feat_dodec,
	feat_count
};

static struct { char *asset; float scale; } feature_model[feat_count] = {
	[feat_tree] = { "tree.obj", 0.5 },
	[feat_tree_small] = { "tree.obj", 0.2 },
	[feat_block] = { "cube.obj", 1.0 },
	[feat_dodec] = { "dodecahedron.obj", 1.0 },
};

static struct {
	struct shader shader[feat_count];
	uint32_t len[feat_count];
	struct darr *feats[feat_count];
} s_feats = { 0 };

bool
render_world_setup_chunks(struct hdarr **chunk_meshes)
{
	enum feature_type feat;
	struct darr *obj_verts = darr_init(sizeof(vec3));
	struct darr *obj_norms = darr_init(sizeof(vec3));
	struct darr *obj_indices = darr_init(sizeof(uint32_t));

	struct shader_spec chunk_spec = {
		.src = {
			{ "chunks.vert", GL_VERTEX_SHADER },
			{ "world.frag", GL_FRAGMENT_SHADER },
		},
		.uniform = { { cu_colors, "colors" } },
		.attribute = {
			{ 3, GL_FLOAT, bt_vbo }, { 3, GL_FLOAT, bt_vbo }, { 1, GL_FLOAT, bt_vbo }
		},
		.static_data = {
			{ chunk_indices, sizeof(uint32_t) * CHUNK_INDICES_LEN, bt_ebo },
		},
	};

	if (!shader_create(&chunk_spec, &chunk_shader)) {
		goto free_exit;
	}

	// create chunk mesh hdarr
	*chunk_meshes = hdarr_init(2048, sizeof(struct point), sizeof(chunk_mesh), NULL);

	glUseProgram(chunk_shader.id);
	glUniform4fv(chunk_shader.uniform[cu_colors], tile_count, (float *)colors.tile_fg);

	/* render features */
	for (feat = 0; feat < feat_count; ++feat) {

		if (!obj_load(feature_model[feat].asset, obj_verts, obj_norms,
			obj_indices, feature_model[feat].scale)) {
			goto free_exit;
		}

		struct shader_spec feat_spec = {
			.src = {
				{ "instanced_model.vert", GL_VERTEX_SHADER },
				{ "world.frag", GL_FRAGMENT_SHADER },
			},
			.attribute = {
				{ 3, GL_FLOAT, bt_vbo }, { 3, GL_FLOAT, bt_nvbo },
				{ 3, GL_FLOAT, bt_ivbo, 1 }, { 3, GL_FLOAT, bt_ivbo, 1 },
				{ 1, GL_FLOAT, bt_ivbo, 1 }
			},
			.static_data = {
				{ darr_raw_memory(obj_verts), darr_size(obj_verts), bt_vbo },
				{ darr_raw_memory(obj_norms), darr_size(obj_norms), bt_nvbo },
				{ darr_raw_memory(obj_indices), darr_size(obj_indices), bt_ebo },
			}
		};

		if (!shader_create(&feat_spec, &s_feats.shader[feat])) {
			goto free_exit;
		}

		s_feats.len[feat] = darr_len(obj_indices);

		s_feats.feats[feat] = darr_init(sizeof(feature_instance));

		darr_clear(obj_verts);
		darr_clear(obj_norms);
		darr_clear(obj_indices);
	}

	darr_destroy(obj_verts);
	darr_destroy(obj_norms);
	darr_destroy(obj_indices);

	return true;

free_exit:
	darr_destroy(obj_verts);
	darr_destroy(obj_norms);
	darr_destroy(obj_indices);
	return false;
}

static void
add_feature(enum tile t, struct chunk_info *ci)
{
	enum feature_type feat_type;
	feature_instance feat = { 0 };

	/* add features */
	switch (t) {
	case tile_wetland_forest:
	case tile_wetland_forest_old:
	case tile_forest_old:
	case tile_forest:
		feat_type = feat_tree;
		break;
	case tile_wetland_forest_young:
	case tile_forest_young:
		feat_type = feat_tree_small;
		break;
	case tile_wood:
	case tile_stone:
		feat_type = feat_block;
		break;
	case tile_shrine:
		feat_type = feat_dodec;
		break;
	default:
		return;
	}

	feat[0] = ci->pos[0] + 0.5;
	feat[1] = ci->pos[1] + 0.5;
	feat[2] = ci->pos[2] + 0.5;
	feat[3] = colors.tile_fg[t][0];
	feat[4] = colors.tile_fg[t][1];
	feat[5] = colors.tile_fg[t][2];
	feat[6] = 1.0;

	darr_push(s_feats.feats[feat_type], feat);
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

					/* add feature */
					add_feature(t, &mesh[i]);
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

void
render_chunks(struct hiface *hf, struct opengl_ui_ctx *ctx, struct hdarr *cms)
{
	enum feature_type feat;
	bool reset_chunks;

	if ((reset_chunks = (ctx->ref_changed || hf->sim->changed.chunks))) {
		/* orphan previous buffer */
		glBindBuffer(GL_ARRAY_BUFFER, chunk_shader.buffer[bt_vbo]);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
			NULL, GL_DYNAMIC_DRAW);

		for (feat = 0; feat < feat_count; ++feat) {
			darr_clear(s_feats.feats[feat]);
		}

		hdarr_clear(cms);
		setup_chunks(hf->sim->w->chunks, ctx, cms);
	}

	shader_use(&chunk_shader);
	shader_check_def_uni(&chunk_shader, ctx);

	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		s_chunk.draw_counts,
		GL_UNSIGNED_INT,
		s_chunk.draw_indices,
		s_chunk.count,
		s_chunk.draw_baseverts);

	/* render features */

	for (feat = 0; feat < feat_count; ++feat) {
		shader_use(&s_feats.shader[feat]);
		shader_check_def_uni(&s_feats.shader[feat], ctx);

		if (reset_chunks) {
			glBindBuffer(GL_ARRAY_BUFFER, s_feats.shader[feat].buffer[bt_ivbo]);

			glBufferData(GL_ARRAY_BUFFER,
				sizeof(feature_instance) * darr_len(s_feats.feats[feat]),
				darr_raw_memory(s_feats.feats[feat]),
				GL_DYNAMIC_DRAW);
		}

		glDrawElementsInstanced(GL_TRIANGLES,
			s_feats.len[feat],
			GL_UNSIGNED_INT,
			(void *)0,
			darr_len(s_feats.feats[feat])
			);
	}

	ctx->reset_chunks = reset_chunks;
}
