#include "posix.h"

#include <string.h>

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/sim/sim.h"
#include "shared/serialize/chunk.h"
#include "shared/sim/tiles.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

static void
handle_new_connection(struct simulation *sim, struct msgr *msgr,
	struct msg_sender *sender)
{
	LOG_I(log_misc, "new connection with id %d", sender->id);

	struct player *p = add_new_player(sim, sender->id);
	// TODO Bad bad bad, p could become invalid!
	sender->usr_ctx = p;

	struct package_ent_updates_ctx peu_ctx = { msgr, sender->addr, .all_alive = true };

	hdarr_for_each(&sim->world->ents, &peu_ctx, check_ent_updates);
}

void
server_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender)
{
	/* L(log_misc, "id:%d:msg:%s", sender->id, inspect_message(mt, _msg)); */

	struct simulation *sim = msgr->usr_ctx;

	switch (mt) {
	case mt_connect:
		handle_new_connection(sim, msgr, sender);
		break;
	case mt_poke:
		break;
	case mt_cursor: {
		struct msg_cursor *msg = _msg;
		struct player *p = sender->usr_ctx;
		p->cursor = msg->cursor;
		p->action = msg->action;
		break;
	}
	case mt_req: {
		struct msg_req *msg = _msg;
		const struct chunk *ck;

		switch (msg->mt) {
		case rmt_chunk:
			ck = get_chunk(&sim->world->chunks, &msg->dat.chunk);
			struct msg_chunk mck;

			fill_ser_chunk(&mck.dat, ck);

			msgr_queue(msgr, mt_chunk, &mck, sender->addr, priority_normal);
			break;
		default:
			assert(false);
			break;
		}
		break;
	}
	case mt_server_cmd: {
		struct msg_server_cmd *msg = _msg;

		switch (msg->cmd) {
		case server_cmd_pause:
			sim->paused = true;
			break;
		case server_cmd_unpause:
			sim->paused = false;
			break;
		case server_cmd_count:
			break;
		}

		break;
	}
	default:
		LOG_W(log_misc, "ignoring unhandled message type: %d", mt);
		break;
	}
}
