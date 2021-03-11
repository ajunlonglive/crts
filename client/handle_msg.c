#include "posix.h"

#include <string.h>

#include "client/client.h"
#include "client/handle_msg.h"
#include "client/input/helpers.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static void
sim_remove_action(struct client *cli, uint8_t id)
{
	L("removing action %d", id);

	/* TODO: re-add assert if this is no longer guaranteed by id type */
	//assert(id < ACTION_HISTORY_SIZE);

	size_t i;

	for (i = 0; i < cli->action_history_len; ++i) {
		if (cli->action_history_order[i] == id) {
			if (i + 1 < cli->action_history_len) {
				memmove(&cli->action_history_order[i],
					&cli->action_history_order[i + 1],
					cli->action_history_len - i
					);
			}

			--cli->action_history_len;
			break;
		}
	}

	cli->action_history[id].type = at_none;

	cli->changed.actions = true;
}

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
			world_despawn(cli->world, msg->id);
			break;
		case emt_pos:
			if ((e = hdarr_get(&cli->world->ents, &msg->id))) {
				e->pos = msg->dat.pos;

				/* vec3 pos = { */
				/* 	e->pos.x, */
				/* 	get_height_at(&cli->world->chunks, &e->pos), */
				/* 	e->pos.y, */
				/* }; */
				/* sound_trigger(cli->sound_ctx, pos); */
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

			vec3 pos = {
				e.pos.x,
				get_height_at(&cli->world->chunks, &e.pos),
				e.pos.y,
			};
			sound_trigger(cli->sound_ctx, pos);

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
	case mt_action:
	{

		struct msg_action *msg = _msg;

		switch (msg->mt) {
		case amt_add:
			break;
		case amt_del:
			sim_remove_action(cli, msg->id);
			break;
		default:
			assert(false);
			break;
		}

		break;
	}
	default:
		assert(false);
	}
}
