#include "posix.h"

#include "server/aggregate_msgs.h"
#include "server/sim/sim.h"
#include "shared/constants/globals.h"
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
	uint8_t modified;

	if (ctx->all_alive) {
		/* all_alive: send all living ents */
		if (e->state & es_killed) {
			return ir_cont;
		}

		modified = e->modified;
	} else if (e->state & es_modified) {
		modified = e->modified;

		e->state &= ~es_modified;
		e->modified = 0;
	} else {
		return ir_cont;
	}

	struct msg_ent msg = {
		.id = e->id,
	};

	if (e->state & es_killed) {
		if (e->state & es_spawned) {
			// we don't need to send a kill message for an ent that
			// was spawned and killed in the same tick
			return ir_cont;
		}
		priority = priority_normal;

		msg.mt = emt_kill;
	} else if (e->state & es_spawned || ctx->all_alive) {
		priority = priority_normal;

		if (!ctx->all_alive) {
			e->state &= ~es_spawned;
		}

		msg.mt = emt_spawn;
		msg.dat.spawn.type = e->type;
		msg.dat.spawn.alignment = e->alignment,
		msg.dat.spawn.pos = e->pos;
	} else {
		priority = priority_normal; // TODO
		msg.mt = emt_update;
		msg.dat.update.modified = modified;

		if (modified & eu_pos) {
			msg.dat.update.pos = e->pos;
		}

		if (modified & eu_alignment) {
			msg.dat.update.alignment = e->alignment;
		}
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
