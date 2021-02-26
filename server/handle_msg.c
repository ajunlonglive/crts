#include "posix.h"

#include <string.h>

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/serialize/chunk.h"
#include "shared/sim/tiles.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

struct find_action_ctx {
	struct sim_action *result;
	msg_addr_t owner;
	uint8_t id;
};

static enum iteration_result
find_action_iterator(void *_ctx, void *_sa)
{
	struct sim_action *sa = _sa;
	struct find_action_ctx *ctx = _ctx;

	if (ctx->owner == sa->owner && ctx->id == sa->owner_handle) {
		ctx->result = sa;
		return ir_done;
	}

	return ir_cont;
}

static struct sim_action *
find_action(struct simulation *sim, msg_addr_t owner, uint8_t id)
{
	struct find_action_ctx ctx = { NULL, owner, id };

	hdarr_for_each(&sim->actions, &ctx, find_action_iterator);

	return ctx.result;
}

static void
handle_new_connection(struct simulation *sim, struct msgr *msgr,
	const struct msg_sender *sender)
{
	LOG_I("new connection with id %d", sender->id);

	add_new_motivator(sim, sender->id);

	struct package_ent_updates_ctx peu_ctx = { msgr, sender->addr, .all_alive = true };

	hdarr_for_each(&sim->world->ents, &peu_ctx, check_ent_updates);
}

void
server_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender)
{
	/* L("id:%d:msg:%s", sender->id, inspect_message(mt, _msg)); */

	struct simulation *sim = msgr->usr_ctx;

	switch (mt) {
	case mt_connect:
		handle_new_connection(sim, msgr, sender);
		break;
	case mt_poke:
		break;
	case mt_req:
	{
		struct msg_req *msg = _msg;
		const struct chunk *ck;

		switch (msg->mt) {
		case rmt_chunk:
			ck = get_chunk(&sim->world->chunks, &msg->dat.chunk);
			struct msg_chunk mck;

			fill_ser_chunk(&mck.dat, ck);

			msgr_queue(msgr, mt_chunk, &mck, sender->addr);
			break;
		default:
			assert(false);
			break;
		}
		break;
	}
	case mt_action:
	{
		struct msg_action *msg = _msg;
		struct sim_action *sact;

		switch (msg->mt) {
		case amt_add:
			if (find_action(sim, sender->addr, msg->id)) {
				/* Don't add duplicate actions */
				/* TODO: handle message duplication at msg_queue lvl */
				return;
			}

			sact = action_add(sim, NULL);

			sact->owner = sender->addr;
			sact->owner_handle = msg->id;

			sact->act.tgt = msg->dat.add.tgt;
			sact->act.type = msg->dat.add.type;
			sact->act.range = msg->dat.add.range;
			/* sact->act.flags = msg.action.flags; */
			/* sact->act.source = msg.action.source; */
			sact->act.motivator = sender->id;
			/* sact->act.workers_requested = msg.action.workers; */

			action_inspect(&sact->act);
			break;
		case amt_del:
			if (!(sact = find_action(sim, sender->addr,
				msg->id))) {
				L("failed to find action %d to delete", msg->id);
				return;
			}

			action_del(sim, sact->act.id);
			break;
		default:
			assert(false);
			break;
		}
		break;
	}
	default:
		LOG_W("ignoring unhandled message type: %d", mt);
		break;
	}
}
