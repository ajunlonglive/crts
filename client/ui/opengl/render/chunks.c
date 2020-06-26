#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/loaders/shader.h"
#include "client/ui/opengl/render/chunks.h"
#include "client/ui/opengl/ui.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

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
	uint32_t id;
	uint32_t vao, vbo, ivbo, ebo;
	uint32_t view, proj, view_pos;
	uint32_t len[feat_count];
	size_t base_index[feat_count];
	size_t base_vert[feat_count];
	size_t base_instance[feat_count];
	struct darr *feats[feat_count];
} s_feats = { 0 };

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

	glUniform4fv(s_chunk.colors, tile_count, (float *)colors.tile_fg);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
		NULL, GL_DYNAMIC_DRAW);

	/* render features */

	struct shader_src feat_src[] = {
		{ "terrain_features.vert", GL_VERTEX_SHADER },
		{ "world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(feat_src, &s_feats.id)) {
		return false;
	}

	glUseProgram(s_feats.id);

	s_feats.view      = glGetUniformLocation(s_feats.id, "view");
	s_feats.proj      = glGetUniformLocation(s_feats.id, "proj");
	s_feats.view_pos  = glGetUniformLocation(s_feats.id, "view_pos");

	glGenVertexArrays(1, &s_feats.vao);
	glGenBuffers(1, &s_feats.vbo);
	glGenBuffers(1, &s_feats.ivbo);
	glGenBuffers(1, &s_feats.ebo);

	enum feature_type feat;
	struct darr *obj_verts[feat_count], *obj_indices[feat_count];
	size_t total_indices = 0, total_vertices = 0;

	for (feat = 0; feat < feat_count; ++feat) {
		obj_verts[feat]   = darr_init(sizeof(vertex_elem));
		obj_indices[feat] = darr_init(sizeof(uint32_t));

		if (!obj_load(feature_model[feat].asset, obj_verts[feat],
			obj_indices[feat], feature_model[feat].scale)) {
			LOG_W("failed to load asset");
			darr_destroy(obj_verts[feat]);
			darr_destroy(obj_indices[feat]);

			obj_verts[feat] = NULL;
			obj_indices[feat] = NULL;
			continue;
		}

		s_feats.base_index[feat] = total_indices;
		s_feats.len[feat] = darr_len(obj_indices[feat]);
		total_indices += s_feats.len[feat];

		s_feats.base_vert[feat] = total_vertices;
		total_vertices += darr_len(obj_verts[feat]);

		s_feats.feats[feat] = darr_init(sizeof(feature_instance));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_feats.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_feats.vbo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * total_indices,
		NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_elem) * total_vertices,
		NULL, GL_DYNAMIC_DRAW);

	total_indices = total_vertices = 0;
	for (feat = 0; feat < feat_count; ++feat) {
		glBufferSubData(GL_ARRAY_BUFFER,
			sizeof(vertex_elem) * total_vertices,
			sizeof(vertex_elem) * darr_len(obj_verts[feat]),
			darr_raw_memory(obj_verts[feat]));

		total_vertices += darr_len(obj_verts[feat]);

		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(uint32_t) * total_indices,
			sizeof(uint32_t) * darr_len(obj_indices[feat]),
			darr_raw_memory(obj_indices[feat]));

		total_indices += darr_len(obj_indices[feat]);

		darr_destroy(obj_verts[feat]);
		darr_destroy(obj_indices[feat]);
	}

	glBindVertexArray(s_feats.vao);

	// position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_elem),
		(void *)0);

	// normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_elem),
		(void *)(3 * sizeof(float)));

	// instance data

	glBindBuffer(GL_ARRAY_BUFFER, s_feats.ivbo);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(feature_instance),
		(void *)0);
	glVertexAttribDivisor(2, 1);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(feature_instance),
		(void *)(3 * sizeof(float)));
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(feature_instance),
		(void *)(6 * sizeof(float)));
	glVertexAttribDivisor(4, 1);

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

	enum feature_type feat_type;
	feature_instance feat = { 0 };
	bool add_feature = false;

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

					/* add features */
					switch (t) {
					case tile_wetland_forest:
					case tile_wetland_forest_old:
					case tile_forest_old:
					case tile_forest:
						feat_type = feat_tree;
						add_feature = true;
						break;
					case tile_wetland_forest_young:
					case tile_forest_young:
						feat_type = feat_tree_small;
						add_feature = true;
						break;
					case tile_wood:
					case tile_stone:
						feat_type = feat_block;
						add_feature = true;
						break;
					case tile_shrine:
						feat_type = feat_dodec;
						add_feature = true;
						break;
					default:
						add_feature = false;
						break;
					}

					if (add_feature) {
						feat[0] = mesh[i].pos[0] + 0.5;
						feat[1] = mesh[i].pos[1] + 0.5;
						feat[2] = mesh[i].pos[2] + 0.5;
						feat[3] = colors.tile_fg[t][0];
						feat[4] = colors.tile_fg[t][1];
						feat[5] = colors.tile_fg[t][2];
						feat[6] = 1.0;

						darr_push(s_feats.feats[feat_type], feat);
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
	enum feature_type feat;

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

		for (feat = 0; feat < feat_count; ++feat) {
			darr_clear(s_feats.feats[feat]);
		}

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

	/* render features */

	glUseProgram(s_feats.id);
	glBindVertexArray(s_feats.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_feats.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_feats.ivbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_feats.proj, 1, GL_TRUE, (float *)ctx->mproj);
		glUniformMatrix4fv(s_feats.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_feats.view_pos, 1, cam.pos);
	}

	if (reset_chunks) {
		size_t feat_len = 0;

		for (feat = 0; feat < feat_count; ++feat) {
			feat_len += darr_len(s_feats.feats[feat]);
		}

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(feature_instance) * feat_len, NULL,
			GL_DYNAMIC_DRAW);

		feat_len = 0;
		for (feat = 0; feat < feat_count; ++feat) {
			glBufferSubData(GL_ARRAY_BUFFER,
				sizeof(feature_instance) * feat_len,
				sizeof(feature_instance) * darr_len(s_feats.feats[feat]),
				darr_raw_memory(s_feats.feats[feat]));

			s_feats.base_instance[feat] = feat_len;
			feat_len += darr_len(s_feats.feats[feat]);
		}
	}

	for (feat = 0; feat < feat_count; ++feat) {
		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
			s_feats.len[feat],
			GL_UNSIGNED_INT,
			(void *)(sizeof(uint32_t) * s_feats.base_index[feat]),
			darr_len(s_feats.feats[feat]),
			s_feats.base_vert[feat],
			s_feats.base_instance[feat]
			);
	}

	return reset_chunks;
}
