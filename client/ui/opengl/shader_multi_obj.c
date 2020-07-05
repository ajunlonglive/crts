#include "posix.h"

#include <assert.h>
#include <string.h>

#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/shader_multi_obj.h"
#include "shared/util/log.h"

typedef float position_data[4];
typedef float lighting_data[3];

bool
shader_create_multi_obj(struct model_spec *ms, size_t mslen,
	struct shader_multi_obj *smo)
{
	bool res = false;
	size_t i, off[4] = { 0 }, old_indices = 0;

	struct darr *obj_verts = darr_init(sizeof(vec3));
	struct darr *obj_norms = darr_init(sizeof(vec3));
	struct darr *obj_indices = darr_init(sizeof(uint32_t));

	struct shader_attrib_spec attribs[COUNT][COUNT] = { 0 };

	enum buffer_type auto_buf = bt_ivbo;

	for (i = 0; i < mslen; ++i) {
		if (!obj_load(ms[i].asset, obj_verts, obj_norms,
			obj_indices, ms[i].scale)) {
			goto free_exit;
		}

		smo->obj_data[i].indices = darr_len(obj_indices) - old_indices;
		old_indices = darr_len(obj_indices);

		smo->obj_data[i].index_offset = off[bt_ebo];

		smo->obj_data[i].position = darr_init(sizeof(position_data));
		smo->obj_data[i].lighting = darr_init(sizeof(lighting_data));

		struct shader_attrib_spec attr[] = {
			{ 3, GL_FLOAT, bt_vbo,       true,  0, off[bt_vbo]  },
			{ 3, GL_FLOAT, bt_nvbo,      false, 0, off[bt_nvbo] },
			{ 3, GL_FLOAT, auto_buf,     true,  1, 0            },
			{ 1, GL_FLOAT, auto_buf,     true,  1, 0            },
			{ 3, GL_FLOAT, auto_buf + 1, false, 1, 0            },
		};

		memcpy(attribs[i], attr, sizeof(struct shader_attrib_spec) * 5);

		off[bt_ebo] = darr_size(obj_indices);
		off[bt_vbo] = darr_size(obj_verts);
		off[bt_nvbo] = darr_size(obj_norms);

		smo->obj_data[i].buf[0] = auto_buf;
		smo->obj_data[i].buf[1] = auto_buf + 1;
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
			{ darr_raw_memory(obj_norms),   darr_size(obj_norms), bt_nvbo },
			{ darr_raw_memory(obj_indices), darr_size(obj_indices), bt_ebo },
		},
	};

	memcpy(spec.attribute, attribs, sizeof(struct shader_attrib_spec) * COUNT * COUNT);

	if (!shader_create(&spec, &smo->shader)) {
		goto free_exit;
	}

	smo->len = mslen;

	res = true;
free_exit:
	darr_destroy(obj_verts);
	darr_destroy(obj_norms);
	darr_destroy(obj_indices);

	return res;
}

void
smo_clear(struct shader_multi_obj *smo)
{
	size_t i;

	for (i = 0; i < smo->len; ++i) {
		darr_clear(smo->obj_data[i].position);
		darr_clear(smo->obj_data[i].lighting);
	}
}

void
smo_push(struct shader_multi_obj *smo, uint32_t i, obj_data data)
{
	darr_push(smo->obj_data[i].position, data);
	darr_push(smo->obj_data[i].lighting, &data[4]);
}

void
smo_upload(struct shader_multi_obj *smo)
{
	uint32_t i;

	for (i = 0; i < smo->len; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER,
			smo->shader.buffer[smo->obj_data[i].buf[0]]);

		glBufferData(GL_ARRAY_BUFFER,
			darr_size(smo->obj_data[i].position),
			darr_raw_memory(smo->obj_data[i].position),
			GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,
			smo->shader.buffer[smo->obj_data[i].buf[1]]);

		glBufferData(GL_ARRAY_BUFFER,
			darr_size(smo->obj_data[i].lighting),
			darr_raw_memory(smo->obj_data[i].lighting),
			GL_DYNAMIC_DRAW);
	}
}

void
smo_draw(struct shader_multi_obj *smo, struct opengl_ui_ctx *ctx)
{
	uint32_t i;

	glUseProgram(smo->shader.id[ctx->pass]);
	shader_check_def_uni(&smo->shader, ctx);

	for (i = 0; i < smo->len; ++i) {
		glBindVertexArray(smo->shader.vao[ctx->pass][i]);

		glDrawElementsInstanced(GL_TRIANGLES,
			smo->obj_data[i].indices,
			GL_UNSIGNED_INT,
			(void *)(smo->obj_data[i].index_offset),
			darr_len(smo->obj_data[i].position)
			);

		ctx->prof.smo_vert_count +=
			smo->obj_data[i].indices * darr_len(smo->obj_data[i].position);
	}
}
