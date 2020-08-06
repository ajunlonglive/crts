#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/color_cfg.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/shader.h"
#include "client/ui/opengl/shader_multi_obj.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

enum ent_model {
	em_cube,
	em_dodec,
	em_deer,
	ent_model_count,
};

static struct model_spec ent_model[ent_model_count][detail_levels] = {
	[em_cube]  = { { "cube.obj", 0.5 }, },
	[em_dodec] = { { "dodecahedron.obj", 0.8 }, },
	[em_deer]  = { { "deer.obj", 0.0012 }, }
};

struct shader_multi_obj ent_shader = { 0 };

bool
render_world_setup_ents(void)
{
	return shader_create_multi_obj(ent_model, ent_model_count, &ent_shader);
}

void
render_ents_setup_frame(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	if (!hf->sim->changed.ents) {
		return;
	}

	struct ent *emem = darr_raw_memory(hdarr_darr(hf->sim->w->ents));
	size_t i, len = hdarr_len(hf->sim->w->ents);
	enum ent_type et;

	smo_clear(&ent_shader);

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

		obj_data info = {
			emem[i].pos.x,
			height,
			emem[i].pos.y,
			1.0,
			colors.ent[color_type][0],
			colors.ent[color_type][1],
			colors.ent[color_type][2],
		};

		enum ent_model em;

		switch (et) {
		case et_deer:
			em = em_deer;
			break;
		default:
			em = em_cube;
		}

		smo_push(&ent_shader, em, info);
	}

	smo_upload(&ent_shader);
}

void
render_ents(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	smo_draw(&ent_shader, ctx);
}
