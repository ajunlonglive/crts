#include "posix.h"

#include <string.h>

#include "client/client.h"
#include "client/handle_msg.h"
#include "client/input/helpers.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

void
client_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender)
{
	struct client *cli = msgr->usr_ctx;
	/* L("id:%d:msg:%s", sender->id, inspect_message(mt, _msg)); */

	switch (mt) {
	case mt_connect:
	case mt_poke:
		break;
	case mt_ent:
	{
		struct msg_ent *msg = _msg;
		struct ent *e;

		switch (msg->mt) {
		case emt_kill:
			if ((e = hdarr_get(&cli->world->ents, &msg->id))) {
				if (e->type != et_elf_corpse) {
					/* if (!cli->sound_triggered) { */
					vec3 pos = {
						e->pos.x,
						get_height_at(&cli->world->chunks, &e->pos),
						e->pos.y,
					};
					sound_trigger(&cli->sound_ctx, pos, audio_asset_die, audio_flag_rand);
					cli->sound_triggered = true;
					/* } */
				}

				world_despawn(cli->world, msg->id);
			} else {
				LOG_W("ignoring kill for nonexistent ent");
			}
			break;
		case emt_pos:
			if ((e = hdarr_get(&cli->world->ents, &msg->id))) {
				e->pos = msg->dat.pos;

				if (!cli->sound_triggered) {
					/* if (rand_chance(1000)) { */
					vec3 pos = {
						e->pos.x,
						get_height_at(&cli->world->chunks, &e->pos),
						e->pos.y,
					};

					enum audio_asset asset;
					switch (get_tile_at(&cli->world->chunks, &e->pos)) {
					case tile_coast:
						asset = audio_asset_step_sand;
						break;
					case tile_old_tree:
					case tile_tree:
					case tile_plain:
						asset = audio_asset_step_grass;
						break;
					case tile_rock:
						asset = audio_asset_step_rock;
						break;
					case tile_ash:
					case tile_fire:
					case tile_dirt:
						asset = audio_asset_step_dirt;
						break;
					default:
						asset = audio_asset_step_sand;
						break;
					}

					sound_trigger(&cli->sound_ctx, pos, asset, audio_flag_rand);
					cli->sound_triggered = true;
				}
			} else {
				LOG_W("ignoring pos for nonexistent ent");
			}
			break;
		case emt_spawn:
		{
			struct ent e = {
				.id = msg->id,
				.alignment = msg->dat.spawn.alignment,
				.pos = msg->dat.spawn.pos,
				.type = msg->dat.spawn.type,
			};

			hdarr_set(&cli->world->ents, &e.id, &e);

			if (!(cli->state & csf_view_initialized)
			    && e.alignment == cli->id) {
				client_init_view(cli, &e.pos);
			}

			if (e.type != et_elf_corpse) {
				/* if (!cli->sound_triggered) { */
				vec3 pos = {
					e.pos.x,
					get_height_at(&cli->world->chunks, &e.pos),
					e.pos.y,
				};
				sound_trigger(&cli->sound_ctx, pos, audio_asset_spawn, audio_flag_rand);
				cli->sound_triggered = true;
				/* } */
			}

			break;
		}
		default:
			assert(false);
		}

		cli->changed.ents = true;
		break;
	}
	case mt_chunk:
	{
		struct msg_chunk *msg = _msg;

		struct chunk ck;
		unfill_ser_chunk(&msg->dat, &ck);

		hdarr_set(&cli->world->chunks.hd, &ck.pos, &ck);

		cli->changed.chunks = true;
		break;
	}
	default:
		LOG_W("recieved unhandled message: id:%d:msg:%s", sender->id, inspect_message(mt, _msg));
		assert(false);
	}
}
