#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/net.h"
#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/net/connection.h"
#include "shared/net/msg_queue.h"
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
struct handle_msgs_ctx {
	struct net_ctx *nx;
	struct simulation *sim;
};

static void
handle_new_connection(struct handle_msgs_ctx *ctx, struct wrapped_message *wm)
{
	const size_t *motp;
	size_t mot;

	L("client id: %d", wm->cm.client_id);

	if ((motp = hash_get(motivators, &wm->cm.client_id)) == NULL) {
		mot = add_new_motivator(ctx->sim);
		hash_set(motivators, &wm->cm.client_id, mot);
	} else {
		mot = *motp;
	}

	struct package_ent_updates_ctx peu_ctx = { ctx->nx, NULL, 0, wm->cx->bit, true };

	hdarr_for_each(ctx->sim->world->ents, &peu_ctx, package_ent_updates);

	wm->cx->motivator = mot;

	send_msg(ctx->nx, server_message_hello, &wm->cx->motivator, wm->cx->bit, 0);

}

static enum iteration_result
handle_msg(void *_ctx, void *_wm)
{
	struct wrapped_message *wm = _wm;
	struct handle_msgs_ctx *ctx = _ctx;
	struct sim_action *sact;
	const struct chunk *ck;

	if (wm->cx->new) {
		handle_new_connection(ctx, wm);
		wm->cx->new = false;
	}

	switch (wm->cm.type) {
	case client_message_poke:
		break;
	case client_message_chunk_req:
		ck = get_chunk(&ctx->sim->world->chunks, &wm->cm.msg.chunk_req.pos);

		send_msg(ctx->nx, server_message_chunk, ck, wm->cx->bit,
			msgf_drop_if_full | msgf_forget);
		break;
	case client_message_action:
		if (wm->cm.msg.action.type) {
			if (find_action(ctx->sim, wm->cx->bit, wm->cm.msg.action.id)) {
				/* Don't add duplicate actions */
				/* TODO: handle message duplaction at msg_queue lvl */
				return ir_cont;
			}

			sact = action_add(ctx->sim, NULL);

			sact->owner = wm->cx->bit;
			sact->owner_handle = wm->cm.msg.action.id;

			sact->act.tgt = wm->cm.msg.action.tgt;
			sact->act.type = wm->cm.msg.action.type;
			sact->act.range = wm->cm.msg.action.range;
			sact->act.flags = wm->cm.msg.action.flags;
			sact->act.source = wm->cm.msg.action.source;
			sact->act.motivator = wm->cx->motivator;
			sact->act.workers_requested = wm->cm.msg.action.workers;

			action_inspect(&sact->act);
		} else {
			if (!(sact = find_action(ctx->sim, wm->cx->bit,
				wm->cm.msg.action.id))) {
				L("failed to find action %d to delete", wm->cm.msg.action.id);
				return ir_cont;
			}

			action_del(ctx->sim, sact->act.id);
		}
		break;
	}

	return ir_cont;
}

void
handle_msgs(struct simulation *sim, struct net_ctx *nx)
{
	struct handle_msgs_ctx ctx = { nx, sim };

	darr_for_each(nx->recvd, &ctx, handle_msg);
	darr_clear(nx->recvd);
}

void
handle_msgs_init(void)
{
	motivators = hash_init(32, 1, sizeof(uint32_t));
}
