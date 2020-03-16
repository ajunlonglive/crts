#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <string.h>

#include "server/net.h"
#include "server/sim/sim.h"
#include "shared/constants/globals.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/alignment.h"
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

struct ent_update_ctx {
	struct net_ctx *nx;
	struct server_message *sm;
	size_t smi;
};

static enum iteration_result
check_ent_updates(void *_ctx, void *_e)
{
	struct ent_update_ctx *ctx = _ctx;
	struct ent *e = _e;
	struct sm_ent *sme;

	if (e->changed) {
		if (ctx->sm == NULL || ctx->smi >= SM_ENT_LEN - 1) {
			if ((ctx->sm = msgq_add(ctx->nx->send,
				ctx->nx->cxs.cx_bits, msgf_must_send)) == NULL) {
				return ir_done;
			}

			sm_init(ctx->sm, server_message_ent, NULL);

			ctx->smi = 0;
		} else {
			++ctx->smi;
		}

		sme = &ctx->sm->msg.ent;

		sme->updates[ctx->smi].id = e->id;
		sme->updates[ctx->smi].type = (e->type << 24) | (e->alignment->max << 16);

		if (e->dead) {
			sme->updates[ctx->smi].type |= eut_kill;
		} else {
			sme->updates[ctx->smi].type |= eut_pos;
			sme->updates[ctx->smi].ud.pos = e->pos;
		}

		e->changed = false;
	}

	return ir_cont;
}

void
aggregate_msgs(struct simulation *sim, struct net_ctx *nx)
{
	if (sim->chunk_date != sim->world->chunks->chunk_date) {
		hdarr_for_each(sim->world->chunks->hd, nx, check_chunk_updates);
		sim->chunk_date = sim->world->chunks->chunk_date;
	}

	struct ent_update_ctx ctx = { nx, NULL, 0 };

	hdarr_for_each(sim->world->ents, &ctx, check_ent_updates);

	broadcast_msg(nx, server_message_world_info, sim->world, msgf_forget);
}
