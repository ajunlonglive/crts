#include "posix.h"

#include <string.h>

#include "client/client.h"
#include "client/handle_msg.h"
#include "client/input_handler.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

static void
update_ent_height(struct client *cli, struct ent *e, int8_t delta)
{
	struct point p = nearest_chunk(&e->pos);
	struct chunk *ck = hdarr_get(&cli->world->chunks.hd, &p);
	if (ck) {
		p = point_sub(&e->pos, &ck->pos);

		ck->ent_height[p.x][p.y] += delta;
	}
}

void
client_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender)
{
	struct client *cli = msgr->usr_ctx;
	/* L(log_net, "id:%d:msg:%s", sender->id, inspect_message(mt, _msg)); */

	switch (mt) {
	case mt_connect:
	case mt_poke:
		break;
	case mt_ent:
	{
		struct msg_ent *msg = _msg;
		uint32_t id = msg->id;
		struct ent *e;

		switch (msg->mt) {
		case emt_kill:
			if ((e = hdarr_get(&cli->world->ents, &id))) {
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

				update_ent_height(cli, e, -1);
			} else {
				LOG_W(log_misc, "ignoring kill for nonexistent ent");
			}
			break;
		case emt_update:
			if (!(e = hdarr_get(&cli->world->ents, &id))) {
				LOG_W(log_misc, "ignoring update for nonexistent ent");
				break;
			}

			if (msg->dat.update.modified & eu_alignment) {
				e->alignment = msg->dat.update.alignment;
			}

			if (msg->dat.update.modified & eu_pos) {
				update_ent_height(cli, e, -1);
				e->pos = msg->dat.update.pos;
				update_ent_height(cli, e, 1);

				if (!cli->sound_triggered) {
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
			}
			break;
		case emt_spawn: {
			struct ent e = {
				.id = id,
				.alignment = msg->dat.spawn.alignment,
				.pos = msg->dat.spawn.pos,
				.type = msg->dat.spawn.type,
			};

			hdarr_set(&cli->world->ents, &id, &e);

			update_ent_height(cli, &e, 1);

			if (!(cli->state & csf_view_initialized)
			    && e.alignment == cli->id) {
				cli->state |= csf_view_initialized;
				cli->view = e.pos;
				cli->cursor = (struct point) { 0, 0 };
				center_cursor(cli, 0);
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
	case mt_chunk: {
		struct msg_chunk *msg = _msg;

		struct chunk ck;
		unfill_ser_chunk(&msg->dat, &ck);

		hdarr_set(&cli->world->chunks.hd, &ck.pos, &ck);

		cli->changed.chunks = true;
		break;
	}
	case mt_server_info: {
		struct msg_server_info *msg = _msg;
		cli->prof.server_fps = msg->fps;
		break;
	}
	default:
		LOG_W(log_misc, "recieved unhandled message: id:%d:msg:%s", sender->id, inspect_message(mt, _msg));
		assert(false);
	}
}
