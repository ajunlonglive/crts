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
	hash_clear(&cli->ents);

	TracyCZoneAutoS;

	hash_clear(&ents_per_tile);
	ctx->stats.friendly_ent_count = 0;
	ctx->stats.live_ent_count = 0;

	struct ent *emem = darr_raw_memory(&cli->world->ents.darr);
	size_t i, len = hdarr_len(&cli->world->ents);

	enum ent_type et;

	smo_clear(&ent_shader);

	float clr[4];

	for (i = 0; i < len; ++i) {
		float pos_diff[3] = {
			emem[i].pos.x - emem[i].real_pos[0],
			emem[i].z - emem[i].real_pos[1],
			emem[i].pos.y - emem[i].real_pos[2],
		};
		vec_scale(pos_diff, 0.3f);
		vec_add(emem[i].real_pos, pos_diff);

		struct point pos = { emem[i].real_pos[0], emem[i].real_pos[2] };

		/* if (!point_in_rect(&pos, &cli->ref.rect)) { */
		/* 	continue; */
		/* } */

		uint64_t hashed = fnv_1a_64(4, (uint8_t *)&emem[i].id);
		float lightness =  ((float)hashed / (float)UINT64_MAX) * 0.2f + 0.8f;

		et = emem[i].type;
		clr[0] = colors.ent[et][0] * lightness;
		clr[1] = colors.ent[et][1] * lightness;
		clr[2] = colors.ent[et][2] * lightness;
		clr[3] = colors.ent[et][3];

		obj_data info = {
			emem[i].real_pos[0], emem[i].real_pos[1], emem[i].real_pos[2], 1.0,
			clr[0], clr[1], clr[2], clr[3],
		};

		smo_push(&ent_shader, em_cube, info);
		struct point3d key = { emem[i].pos.x, emem[i].z, emem[i].pos.y };
		hash_set(&cli->ents, &key, (uint64_t)(&emem[i]));
	}

	smo_upload(&ent_shader);

	TracyCZoneAutoE;
}

void
render_ents(struct client *cli, struct gl_ui_ctx *ctx)
{
	smo_draw(&ent_shader, ctx);
}
