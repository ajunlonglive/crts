#include "posix.h"

#include "client/client.h"
#include "client/ui/gl/colors.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/ents.h"
#include "client/ui/gl/shader.h"
#include "client/ui/gl/shader_multi_obj.h"
#include "shared/math/hash.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "tracy.h"

enum ent_model {
	em_cube,
	em_cube_resource,
	em_dodec,
	em_deer,
	ent_model_count,
};

static struct model_spec ent_model[ent_model_count][detail_levels] = {
	[em_cube]  = { { "cube.obj", 1.0 }, },
	[em_cube_resource]  = { { "cube.obj", 0.5 }, },
	[em_dodec] = { { "dodecahedron.obj", 0.8 }, },
	[em_deer]  = { { "deer.obj", 0.0012 }, }
};

static struct shader_multi_obj ent_shader = { 0 };

static struct hash ents_per_tile = { 0 };

bool
render_world_setup_ents(void)
{
	hash_init(&ents_per_tile, 2048, sizeof(struct point));

	return shader_create_multi_obj(ent_model, ent_model_count, &ent_shader);
}

void
render_ents_setup_frame(struct client *cli, struct gl_ui_ctx *ctx)
{
	/* if (!cli->changed.ents) { */
	/* 	return; */
	/* } */

	TracyCZoneAutoS;

	hash_clear(&ents_per_tile);
	ctx->stats.friendly_ent_count = 0;
	ctx->stats.live_ent_count = 0;

	struct ent *emem = darr_raw_memory(&cli->world->ents.darr);
	size_t i, len = hdarr_len(&cli->world->ents);
	uint64_t *cnt;
	float height;
	uint16_t ent_height;

	enum ent_type et;

	smo_clear(&ent_shader);

	float clr[4];

	for (i = 0; i < len; ++i) {
		if (!point_in_rect(&emem[i].pos, &ctx->ref)) {
			continue;
		}

		struct point p = nearest_chunk(&emem[i].pos);
		struct chunk *ck = hdarr_get(&cli->world->chunks.hd, &p);

		if (ck) {
			p = point_sub(&emem[i].pos, &ck->pos);
			if (ck->tiles[p.x][p.y] <= tile_sea) {
				height = -2.0;
			} else {
				height = 0.5 + ck->heights[p.x][p.y];
			}

			if ((cnt = hash_get(&ents_per_tile, &emem[i].pos))) {
				ent_height = *cnt;
				*cnt += 1;
			} else {
				hash_set(&ents_per_tile, &emem[i].pos, 1);
				ent_height = 0;
			}

			if (ck->ent_height[p.x][p.y] - 1 > ent_height) {
				/* continue; */
			}

			height += ent_height;
		} else {
			height = 0.0f;
		}

		et = emem[i].type;

		uint64_t hashed = fnv_1a_64(4, (uint8_t *)&emem[i].id);
		float lightness =  ((float)hashed / (float)UINT64_MAX) * 0.2f + 0.8f;

		clr[0] = colors.ent[et][0] * lightness;
		clr[1] = colors.ent[et][1] * lightness;
		clr[2] = colors.ent[et][2] * lightness;
		clr[3] = colors.ent[et][3];

		obj_data info = {
			emem[i].pos.x, height, emem[i].pos.y, 1.0,
			clr[0], clr[1], clr[2], clr[3],
		};

		smo_push(&ent_shader, em_cube, info);
	}

	smo_upload(&ent_shader);

	TracyCZoneAutoE;
}

void
render_ents(struct client *cli, struct gl_ui_ctx *ctx)
{
	smo_draw(&ent_shader, ctx);
}
