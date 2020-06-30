#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/loaders/shader.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/shader.h"
#include "shared/util/log.h"

typedef float ent_info[7];
struct shader ent_shader[ent_type_count] = { 0 };
struct darr *entity_data[ent_type_count];
size_t indices_len[ent_type_count];

static struct { char *asset; float scale; } ent_model[ent_type_count] = {
	[et_none]           = { "cube.obj", 0.5 },
	[et_worker]         = { "cube.obj", 0.5 },
	[et_elf_corpse]     = { "cube.obj", 0.5 },
	[et_deer]           = { "cube.obj", 0.5 },
	[et_fish]           = { "cube.obj", 0.5 },
	[et_vehicle_boat]   = { "cube.obj", 0.5 },
	[et_resource_wood]  = { "dodecahedron.obj", 0.25 },
	[et_resource_meat]  = { "dodecahedron.obj", 0.25 },
	[et_resource_rock]  = { "dodecahedron.obj", 0.25 },
	[et_resource_crop]  = { "dodecahedron.obj", 0.25 },
};

bool
render_world_setup_ents(void)
{
	struct darr *obj_verts = darr_init(sizeof(vec3));
	struct darr *obj_norms = darr_init(sizeof(vec3));
	struct darr *obj_indices = darr_init(sizeof(uint32_t));
	enum ent_type et;

	for (et = 0; et < ent_type_count; ++et) {
		entity_data[et] = darr_init(sizeof(ent_info));

		if (!obj_load(ent_model[et].asset, obj_verts, obj_norms,
			obj_indices, ent_model[et].scale)) {
			goto free_exit;
		}

		indices_len[et] = darr_len(obj_indices);

		struct shader_spec ent_spec = {
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

		if (!shader_create(&ent_spec, &ent_shader[et])) {
			goto free_exit;
		}

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
setup_ents(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(hf->sim->w->ents));
	size_t i, len = hdarr_len(hf->sim->w->ents);
	enum ent_type et;

	for (i = 0; i < len; ++i) {
		if (!point_in_rect(&emem[i].pos, &ctx->ref)) {
			continue;
		}

		struct point p = nearest_chunk(&emem[i].pos);
		struct chunk *ck = hdarr_get(hf->sim->w->chunks->hd, &p);

		float height = 0.0;
		uint32_t color_type = et = emem[i].type;

		if (ck) {
			p = point_sub(&emem[i].pos, &ck->pos);
			if (ck->tiles[p.x][p.y] <= tile_water) {
				height = -2.0;
			} else {
				height = 0.5 + ck->heights[p.x][p.y];
			}
		}

		if (et == et_worker) {
			if (emem[i].alignment == hf->sim->assigned_motivator) {
				color_type = et_elf_friend;
			} else {
				color_type = et_elf_foe;
			}
		}

		ent_info info = {
			emem[i].pos.x,
			height,
			emem[i].pos.y,
			colors.ent[color_type][0],
			colors.ent[color_type][1],
			colors.ent[color_type][2],
			1.0
		};

		darr_push(entity_data[et], info);
	}
}

void
render_ents(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	enum ent_type et;

	if (hf->sim->changed.ents) {
		for (et = 0; et < ent_type_count; ++et) {
			darr_clear(entity_data[et]);
		}

		setup_ents(hf, ctx);

		for (et = 0; et < ent_type_count; ++et) {
			glBindBuffer(GL_ARRAY_BUFFER, ent_shader[et].buffer[bt_ivbo]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(ent_info) * darr_len(entity_data[et]),
				darr_raw_memory(entity_data[et]), GL_DYNAMIC_DRAW);
		}
	}

	for (et = 0; et < ent_type_count; ++et) {
		shader_use(&ent_shader[et]);
		shader_check_def_uni(&ent_shader[et], ctx);

		glDrawElementsInstanced(
			GL_TRIANGLES,
			indices_len[et],
			GL_UNSIGNED_INT,
			(void *)0,
			darr_len(entity_data[et]));
	}
}
