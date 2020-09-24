#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <string.h>

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/net.h"
#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/net/connection.h"
#include "shared/net/msg_queue.h"
#include "shared/serialize/chunk.h"
#include "shared/sim/tiles.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

struct find_action_ctx {
	struct sim_action *result;
	cx_bits_t owner;
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
find_action(struct simulation *sim, cx_bits_t owner, uint8_t id)
{
	struct find_action_ctx ctx = { NULL, owner, id };

	hdarr_for_each(sim->actions, &ctx, find_action_iterator);

	return ctx.result;
}

static struct hash *motivators;

static void
handle_new_connection(struct simulation *sim, struct net_ctx *nx,
	struct connection *cx)
{
	const size_t *motp;
	size_t mot;

	/* TODO: stop faking this */
	uint32_t client_id = 0;
	L("client id: %d", client_id);

	if ((motp = hash_get(motivators, &client_id)) == NULL) {
		mot = add_new_motivator(sim);
		hash_set(motivators, &client_id, mot);
	} else {
		mot = *motp;
	}

	struct package_ent_updates_ctx peu_ctx = { nx, cx->bit, .all_alive = true };

	hdarr_for_each(sim->world->ents, &peu_ctx, check_ent_updates);

	cx->motivator = mot;
}

void
handle_msg(struct net_ctx *nx, enum message_type mt, void *_msg,
	struct connection *cx)
{
	L("msg:%s", inspect_message(mt, _msg));

	struct simulation *sim = nx->usr_ctx;

	if (cx->new) {
		L("new conenection");
		handle_new_connection(sim, nx, cx);
		cx->new = false;
	}

	switch (mt) {
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

			fill_msg_chunk(&mck, ck);

			queue_msg(nx, mt_chunk, &mck, cx->bit,
				msgf_drop_if_full | msgf_forget);
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
			if (find_action(sim, cx->bit, msg->id)) {
				/* Don't add duplicate actions */
				/* TODO: handle message duplaction at msg_queue lvl */
				return;
			}

			sact = action_add(sim, NULL);

			sact->owner = cx->bit;
			sact->owner_handle = msg->id;

			/* sact->act.tgt = msg.action.tgt; */
			sact->act.type = msg->dat.add.type;
			sact->act.range = msg->dat.add.range;
			/* sact->act.flags = msg.action.flags; */
			/* sact->act.source = msg.action.source; */
			sact->act.motivator = cx->motivator;
			/* sact->act.workers_requested = msg.action.workers; */

			action_inspect(&sact->act);
			break;
		case amt_del:
			if (!(sact = find_action(sim, cx->bit,
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

void
handle_msgs_init(void)
{
	motivators = hash_init(32, 1, sizeof(uint32_t));
}
