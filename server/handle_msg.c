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
	struct action *act;
	const struct chunk *ck;

	if (wm->cx->new) {
		handle_new_connection(ctx, wm);
		wm->cx->new = false;
	}

	switch (wm->cm.type) {
	case client_message_poke:
		break;
	case client_message_chunk_req:
		ck = get_chunk(ctx->sim->world->chunks, &wm->cm.msg.chunk_req.pos);

		send_msg(ctx->nx, server_message_chunk, ck, wm->cx->bit,
			msgf_drop_if_full | msgf_forget);
		break;
	case client_message_action:
		act = &action_add(ctx->sim, NULL)->act;
		act->motivator = wm->cx->motivator;
		act->type = wm->cm.msg.action.type;
		act->workers_requested = wm->cm.msg.action.workers;
		act->range = wm->cm.msg.action.range;
		act->tgt = wm->cm.msg.action.tgt;

		action_inspect(act);
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
