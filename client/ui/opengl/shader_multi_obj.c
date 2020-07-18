#include "posix.h"

#include <assert.h>
#include <string.h>

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/shader_multi_obj.h"
#include "shared/util/log.h"

typedef float model[7];

bool
shader_create_multi_obj(struct model_spec ms[][detail_levels], size_t mslen,
	struct shader_multi_obj *smo)
{
	bool res = false;
	size_t i, off[4] = { 0 }, old_indices = 0;
	uint32_t vao = 0;

	struct darr *obj_verts = darr_init(sizeof(float) * 6);
	struct darr *obj_indices = darr_init(sizeof(uint32_t));

	struct shader_attrib_spec attribs[COUNT][COUNT] = { 0 };

	enum buffer_type auto_buf = bt_ivbo;
	enum level_of_detail lod;

	for (i = 0; i < mslen; ++i) {
		for (lod = 0; lod < detail_levels; ++lod) {
			if (!ms[i][lod].asset) {
				assert(lod > 0);

				smo->obj_data[i].vao[lod] = smo->obj_data[i].vao[lod - 1];
				smo->obj_data[i].indices[lod] = smo->obj_data[i].indices[lod - 1];
				smo->obj_data[i].index_offset[lod] = smo->obj_data[i].index_offset[lod - 1];

				continue;
			}

			if (!obj_load(ms[i][lod].asset, obj_verts, obj_indices,
				ms[i][lod].scale)) {
				goto free_exit;
			}

			smo->obj_data[i].indices[lod] = darr_len(obj_indices) - old_indices;
			old_indices = darr_len(obj_indices);

			smo->obj_data[i].index_offset[lod] = off[bt_ebo];

			struct shader_attrib_spec attr[] = {
				{ 3, GL_FLOAT, bt_vbo,       true,  0, off[bt_vbo]  },
				{ 3, GL_FLOAT, bt_vbo,       false, 0, off[bt_vbo]  },
				{ 3, GL_FLOAT, auto_buf,     true,  1, 0            },
				{ 1, GL_FLOAT, auto_buf,     true,  1, 0            },
				{ 3, GL_FLOAT, auto_buf,     false, 1, 0            },
			};

			memcpy(attribs[vao], attr, sizeof(struct shader_attrib_spec) * 5);

			off[bt_ebo] = darr_size(obj_indices);
			off[bt_vbo] = darr_size(obj_verts);

			smo->obj_data[i].vao[lod] = vao;
			++vao;
		}

		smo->obj_data[i].model = darr_init(sizeof(model));

		smo->obj_data[i].buf = auto_buf;
		auto_buf += 2;

		assert(auto_buf < 32);
	}

	struct shader_spec spec = {
		.src = {
			[rp_final] = {
				{ "instanced_model.vert", GL_VERTEX_SHADER },
				{ "world.frag", GL_FRAGMENT_SHADER }
			},
			[rp_depth] = {
				{ "instanced_model_depth.vert", GL_VERTEX_SHADER },
				{ "empty.frag", GL_FRAGMENT_SHADER }
			},
		},
		.static_data = {
			{ darr_raw_memory(obj_verts),   darr_size(obj_verts), bt_vbo },
			{ darr_raw_memory(obj_indices), darr_size(obj_indices), bt_ebo },
		},
		.interleaved = true,
	};

	memcpy(spec.attribute, attribs, sizeof(struct shader_attrib_spec) * COUNT * COUNT);

	if (!shader_create(&spec, &smo->shader)) {
		goto free_exit;
	}

	smo->len = mslen;

	res = true;
free_exit:
	darr_destroy(obj_verts);
	/* darr_destroy(obj_norms); */
	darr_destroy(obj_indices);

	return res;
}

void
smo_clear(struct shader_multi_obj *smo)
{
	size_t i;

	for (i = 0; i < smo->len; ++i) {
		darr_clear(smo->obj_data[i].model);
	}
}

void
smo_push(struct shader_multi_obj *smo, uint32_t i, obj_data data)
{
	darr_push(smo->obj_data[i].model, data);
}

void
smo_upload(struct shader_multi_obj *smo)
{
	uint32_t i;

	for (i = 0; i < smo->len; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER,
			smo->shader.buffer[smo->obj_data[i].buf]);

		glBufferData(GL_ARRAY_BUFFER,
			darr_size(smo->obj_data[i].model),
			darr_raw_memory(smo->obj_data[i].model),
			GL_DYNAMIC_DRAW);
	}
}

void
smo_draw(struct shader_multi_obj *smo, struct opengl_ui_ctx *ctx)
{
	uint32_t i;

	glUseProgram(smo->shader.id[ctx->pass]);
	shader_check_def_uni(&smo->shader, ctx);

	enum level_of_detail lod;

	for (i = 0; i < smo->len; ++i) {
		if (!darr_len(smo->obj_data[i].model)) {
			continue;
		}

		lod = darr_len(smo->obj_data[i].model) > 2400 ? lod_1 : lod_0;

		glBindVertexArray(smo->shader.vao[ctx->pass][smo->obj_data[i].vao[lod]]);

		glDrawElementsInstanced(GL_TRIANGLES,
			smo->obj_data[i].indices[lod],
			GL_UNSIGNED_INT,
			(void *)(smo->obj_data[i].index_offset[lod]),
			darr_len(smo->obj_data[i].model)
			);

		ctx->prof.smo_vert_count +=
			smo->obj_data[i].indices[lod] * darr_len(smo->obj_data[i].model);
	}
}
