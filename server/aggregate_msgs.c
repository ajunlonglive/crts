#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <string.h>

#include "server/aggregate_msgs.h"
#include "server/net.h"
#include "server/sim/action.h"
#include "server/sim/sim.h"
#include "shared/constants/globals.h"
#include "shared/net/net_ctx.h"

#include "shared/sim/ent.h"
#include "shared/util/log.h"

static enum iteration_result
check_chunk_updates(void *_nx, void *_c)
{
	struct net_ctx *nx = _nx;
	struct chunk *ck = _c;

	if (ck->touched_this_tick) {
		broadcast_msg(nx, server_message_chunk, ck, 0);
		ck->touched_this_tick = false;
	}

	return ir_cont;
}

enum iteration_result
package_ent_updates(void *_ctx, void *_e)
{
	struct package_ent_updates_ctx *ctx = _ctx;
	struct ent *e = _e;
	struct sm_ent *sme;

	if (ctx->all_alive && !(e->state & es_killed)) {
		// nothing here
	} else if (!ctx->all_alive && (e->state & es_modified)) {
		e->state &= ~es_modified;
	} else {
		return ir_cont;
	}

	if (ctx->sm == NULL || ctx->smi >= SM_ENT_LEN - 1) {
		if ((ctx->sm = msgq_add(ctx->nx->send,
			ctx->dest, msgf_must_send)) == NULL) {
			return ir_done;
		}

		sm_init(ctx->sm, server_message_ent, NULL);

		ctx->smi = 0;
	} else {
		++ctx->smi;
	}

	sme = &ctx->sm->msg.ent;

	sme->updates[ctx->smi].id = e->id;
	sme->updates[ctx->smi].ent_type = e->type;
	sme->updates[ctx->smi].alignment = e->alignment;

	if (e->state & es_killed) {
		sme->updates[ctx->smi].type = eut_kill;
	} else {
		sme->updates[ctx->smi].type = eut_pos;
		sme->updates[ctx->smi].ud.pos = e->pos;
	}

	return ir_cont;
}

static enum iteration_result
check_action_updates(void *_nx, void *_sa)
{
	struct sim_action *sa = _sa;
	struct net_ctx *nx = _nx;

	if (!sa->deleted) {
		return ir_cont;
	}

	L("sending deleted msg");
	send_msg(nx, server_message_rem_action, &sa->owner_handle, sa->owner, 0);

	return ir_cont;
}

void
aggregate_msgs(struct simulation *sim, struct net_ctx *nx)
{
	if (sim->chunk_date != sim->world->chunks->chunk_date) {
		hdarr_for_each(sim->world->chunks->hd, nx, check_chunk_updates);
		sim->chunk_date = sim->world->chunks->chunk_date;
	}

	struct package_ent_updates_ctx ctx = { nx, NULL, 0, nx->cxs.cx_bits, false };

	hdarr_for_each(sim->world->ents, &ctx, package_ent_updates);

	hdarr_for_each(sim->actions, nx, check_action_updates);

	broadcast_msg(nx, server_message_world_info, sim->world, msgf_forget);
}
