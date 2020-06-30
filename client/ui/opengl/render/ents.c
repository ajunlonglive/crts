#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/loaders/shader.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/shader.h"
#include "shared/util/log.h"

typedef float ent_info[7];
struct shader ent_shader = { 0 };
struct darr *entity_data;
size_t indices_len;

bool
render_world_setup_ents(void)
{
	struct darr *obj_verts = darr_init(sizeof(vertex_elem));
	struct darr *obj_indices = darr_init(sizeof(uint32_t));
	entity_data = darr_init(sizeof(ent_info));

	if (!obj_load("cube.obj", obj_verts, obj_indices, 1.0f)) {
		LOG_W("failed to load asset");
		goto free_exit;
	}

	indices_len = darr_len(obj_indices);

	struct shader_spec ent_spec = {
		.src = {
			{ "ents.vert", GL_VERTEX_SHADER },
			{ "world.frag", GL_FRAGMENT_SHADER },
		},
		.attribute = {
			{ 3, GL_FLOAT }, { 3, GL_FLOAT },
			{ 3, GL_FLOAT, true }, { 3, GL_FLOAT, true }, { 1, GL_FLOAT, true }
		},
		.object = {
			.indices_len = sizeof(uint32_t) * darr_len(obj_indices),
			.indices = darr_raw_memory(obj_indices),
			.verts_len = sizeof(vertex_elem) * darr_len(obj_verts),
			.verts = darr_raw_memory(obj_verts),
		},
	};

	if (!shader_create(&ent_spec, &ent_shader)) {
		goto free_exit;
	}

	darr_destroy(obj_verts);
	darr_destroy(obj_indices);

	return true;
free_exit:
	darr_destroy(obj_verts);
	darr_destroy(obj_indices);

	return false;
}

void
render_ents(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(hf->sim->w->ents));
	size_t i, len = hdarr_len(hf->sim->w->ents);

	for (i = 0; i < len; ++i) {
		if (!point_in_rect(&emem[i].pos, &ctx->ref)) {
			continue;
		}

		struct point p = nearest_chunk(&emem[i].pos);
		struct chunk *ck = hdarr_get(hf->sim->w->chunks->hd, &p);

		float height = 0.0;
		uint32_t type = emem[i].type;

		if (ck) {
			p = point_sub(&emem[i].pos, &ck->pos);
			if (ck->tiles[p.x][p.y] <= tile_water) {
				height = -2.0;
			} else {
				height = 0.5 + ck->heights[p.x][p.y];
			}
		}

		if (type == et_worker) {
			if (emem[i].alignment == hf->sim->assigned_motivator) {
				type = et_elf_friend;
			} else {
				type = et_elf_foe;
			}
		}

		ent_info info = {
			emem[i].pos.x,
			height,
			emem[i].pos.y,
			colors.ent[type][0],
			colors.ent[type][1],
			colors.ent[type][2],
			0.5
		};

		darr_push(entity_data, info);
	}

	glBindBuffer(GL_ARRAY_BUFFER, ent_shader.buffer[bt_ivbo]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ent_info) * darr_len(entity_data),
		darr_raw_memory(entity_data), GL_DYNAMIC_DRAW);

	shader_use(&ent_shader);
	shader_check_cam(&ent_shader, ctx);

	glDrawElementsInstanced(
		GL_TRIANGLES,
		indices_len,
		GL_UNSIGNED_INT,
		(void *)0,
		darr_len(entity_data));

	darr_clear(entity_data);
}
