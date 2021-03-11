#include "posix.h"

#include "server/aggregate_msgs.h"
/* #include "server/net.h" */
#include "server/sim/sim.h"
#include "shared/constants/globals.h"
/* #include "shared/net/net_ctx.h" */
#include "shared/serialize/chunk.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"
#include "tracy.h"

static enum iteration_result
check_chunk_updates(void *_msgr, void *_c)
{
	struct msgr *msgr = _msgr;
	struct chunk *ck = _c;

	if (ck->touched_this_tick) {
		struct msg_chunk msg;
		fill_ser_chunk(&msg.dat, ck);
		msgr_queue(msgr, mt_chunk, &msg, 0, priority_normal);
		ck->touched_this_tick = false;
	}

	return ir_cont;
}

enum iteration_result
check_ent_updates(void *_ctx, void *_e)
{
	struct package_ent_updates_ctx *ctx = _ctx;
	struct ent *e = _e;
	enum msg_priority_type priority;

	/* unconditionally skip phantoms */
	if (gcfg.ents[e->type].phantom) {
		return ir_cont;
	} else if (ctx->all_alive) {
		/* all_alive: send all living ents */
		if (e->state & es_killed) {
			return ir_cont;
		}
	} else if (e->state & es_modified) {
		e->state &= ~es_modified;
	} else {
		return ir_cont;
	}

	struct msg_ent msg = {
		.id = e->id,
	};

	if (e->state & es_killed) {
		priority = priority_normal;

		msg.mt = emt_kill;
	} else if (e->state & es_spawned || ctx->all_alive) {
		priority = priority_normal;

		if (!ctx->all_alive) {
			e->state &= ~es_spawned;
		}

		msg.mt = emt_spawn;
		msg.dat.spawn.type = e->type;
		msg.dat.spawn.alignment = e->alignment;
		msg.dat.spawn.pos = e->pos;
	} else {
		priority = priority_dont_resend;

		msg.mt = emt_pos;
		msg.dat.pos = e->pos;
	};

	msgr_queue(ctx->msgr, mt_ent, &msg, ctx->dest, priority);
	return ir_cont;
}

void
aggregate_msgs(struct simulation *sim, struct msgr *msgr)
{
	TracyCZoneAutoS;
	if (sim->chunk_date != sim->world->chunks.chunk_date) {
		hdarr_for_each(&sim->world->chunks.hd, msgr, check_chunk_updates);
		sim->chunk_date = sim->world->chunks.chunk_date;
	}

	struct package_ent_updates_ctx ctx = { msgr, 0, false };

	hdarr_for_each(&sim->world->ents, &ctx, check_ent_updates);

	TracyCZoneAutoE;
}
