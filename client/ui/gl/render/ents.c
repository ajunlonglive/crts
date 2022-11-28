#include "posix.h"

#include <math.h>

#include "client/client.h"
#include "client/ui/gl/colors.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/ents.h"
#include "client/ui/gl/shader.h"
#include "client/ui/gl/shader_multi_obj.h"
#include "shared/math/hash.h"
#include "shared/sim/ent_buckets.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "tracy.h"

enum ent_model_flat {
	em_cube,
	ent_model_flat_count,
};

enum ent_model_smooth {
	em_sphere,
	ent_model_smooth_count,
};

static struct model_spec ent_model_flat[ent_model_flat_count][detail_levels] = {
	[em_cube]  = { { "cube.obj", 1.0 }, },
};

static struct model_spec ent_model_smooth[ent_model_flat_count][detail_levels] = {
	[em_sphere]  = { { "sphere.obj", 10.0 }, },
};

static struct shader_multi_obj ent_shader_flat = { 0 };
static struct shader_multi_obj ent_shader_smooth = { 0 };

bool
render_world_setup_ents(void)
{
	return shader_create_multi_obj(ent_model_flat, ent_model_flat_count, &ent_shader_flat, true)
	       && shader_create_multi_obj(ent_model_smooth, ent_model_smooth_count, &ent_shader_smooth, false);
}

static enum iteration_result
determine_visibility(void *_ctx, struct ent *e)
{
	uint32_t *ctx = _ctx;
	++*ctx;
	return ir_cont;
}

static void
push_ents(struct client *cli, struct gl_ui_ctx *ctx, bool transparent)
{

	struct ent *emem = darr_raw_memory(&cli->world->ents.darr);
	size_t i, len = hdarr_len(&cli->world->ents);

	enum ent_type et;

	float clr[4];

	bool moving;

	for (i = 0; i < len; ++i) {
		et = emem[i].type;
		if (transparent) {
			if (colors.ent[et][3] == 1.0f) {
				continue;
			}
		} else {
			if (colors.ent[et][3] < 1.0f) {
				continue;
			}
		}

		float pos_diff[3] = {
			emem[i].pos.x - emem[i].real_pos[0],
			emem[i].z - emem[i].real_pos[1],
			emem[i].pos.y - emem[i].real_pos[2],
		};
		moving = fabsf(pos_diff[0] + pos_diff[1] + pos_diff[2]) > 0.01f;
		vec_scale(pos_diff, 0.3f);
		vec_add(emem[i].real_pos, pos_diff);

		struct point pos = { emem[i].real_pos[0], emem[i].real_pos[2] };

		if (!point_in_rect(&pos, &cli->ref.rect)) {
			continue;
		}

		if (!transparent && !moving) {
			uint32_t count = 0;
			for_each_ent_adjacent_to(&cli->ents, &emem[i], &count, determine_visibility);
			if (count >= 6) {
				continue;
			}
		}

		uint64_t hashed = fnv_1a_64(4, (uint8_t *)&emem[i].id);
		float lightness =  ((float)hashed / (float)UINT64_MAX) * 0.2f + 0.8f;

		clr[0] = colors.ent[et][0] * lightness;
		clr[1] = colors.ent[et][1] * lightness;
		clr[2] = colors.ent[et][2] * lightness;
		clr[3] = colors.ent[et][3];

		obj_data info = {
			emem[i].real_pos[0], emem[i].real_pos[1], emem[i].real_pos[2], 1.0,
			clr[0], clr[1], clr[2], clr[3],
		};

		if (et == et_dampener || et == et_accelerator) {
			smo_push(&ent_shader_smooth, em_sphere, info);
		} else {
			smo_push(&ent_shader_flat, em_cube, info);
		}

	}
}

void
render_ents_setup_frame(struct client *cli, struct gl_ui_ctx *ctx)
{
	TracyCZoneAutoS;

	hash_clear(&cli->ents);
	ctx->stats.friendly_ent_count = 0;
	ctx->stats.live_ent_count = 0;

	smo_clear(&ent_shader_flat);
	smo_clear(&ent_shader_smooth);

	struct ent *emem = darr_raw_memory(&cli->world->ents.darr);
	size_t i, len = hdarr_len(&cli->world->ents);
	for (i = 0; i < len; ++i) {
		struct point3d key = { emem[i].pos.x, emem[i].pos.y, emem[i].z, };
		/* L(log_cli, "setting %d, %d, %d", key.x, key.y, key.z); */
		hash_set(&cli->ents, &key, (uint64_t)(&emem[i]));
	}

	push_ents(cli, ctx, false);
	push_ents(cli, ctx, true);

	smo_upload(&ent_shader_flat);
	smo_upload(&ent_shader_smooth);

	TracyCZoneAutoE;
}

void
render_ents(struct client *cli, struct gl_ui_ctx *ctx)
{
	smo_draw(&ent_shader_flat, ctx);
	smo_draw(&ent_shader_smooth, ctx);
}
