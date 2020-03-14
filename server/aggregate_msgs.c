#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "server/net.h"
#include "server/sim/sim.h"
#include "shared/constants/globals.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/ent.h"

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

static enum iteration_result
check_ent_updates(void *_nx, void *_e)
{
	struct net_ctx *nx = _nx;
	struct ent *e = _e;

	if (e->changed) {
		if (e->dead) {
			broadcast_msg(nx, server_message_kill_ent, &e->id, 0);
		} else {
			broadcast_msg(nx, server_message_ent, e,
				gcfg.ents[e->type].animate ? 0 : msgf_dont_overwrite);
			e->changed = false;
		}
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

	hdarr_for_each(sim->world->ents, nx, check_ent_updates);

	broadcast_msg(nx, server_message_world_info, sim->world, msgf_forget);
}
