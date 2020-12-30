#include "posix.h"

#include <string.h>

#include "client/sim.h"
#include "client/hiface.h"
#include "client/handle_msg.h"
/* #include "shared/net/net_ctx.h" */
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static void
sim_remove_action(struct c_simulation *sim, uint8_t id)
{
	L("removing action %d", id);

	/* TODO: re-add assert if this is no longer guaranteed by id type */
	//assert(id < ACTION_HISTORY_SIZE);

	size_t i;

	for (i = 0; i < sim->action_history_len; ++i) {
		if (sim->action_history_order[i] == id) {
			if (i + 1 < sim->action_history_len) {
				memmove(&sim->action_history_order[i],
					&sim->action_history_order[i + 1],
					sim->action_history_len - i
					);
			}

			--sim->action_history_len;
			break;
		}
	}

	sim->action_history[id].type = at_none;

	sim->changed.actions = true;
}

void
client_handle_msg(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *_)
{
	struct c_simulation *sim = msgr->usr_ctx;
	/* L("msg:%s", inspect_message(mt, _msg)); */

	switch (mt) {
	case mt_poke:
		break;
	case mt_ent:
	{
		struct msg_ent *msg = _msg;
		struct ent *e;

		switch (msg->mt) {
		case emt_kill:
			world_despawn(sim->w, msg->id);
			break;
		case emt_pos:
			if ((e = hdarr_get(&sim->w->ents, &msg->id))) {
				e->pos = msg->dat.pos;
			} else {
				LOG_W("ignoring pos for nonexistent ent");
			}
			break;
		case emt_spawn:
		{
			struct ent e = {
				.id = msg->id,
				.alignment = msg->dat.spawn.alignment,
				.pos = msg->dat.spawn.pos,
				.type = msg->dat.spawn.type,
			};

			hdarr_set(&sim->w->ents, &e.id, &e);
			break;
		}
		default:
			assert(false);
		}

		sim->changed.ents = true;
		break;
	}
	case mt_chunk:
	{
		struct msg_chunk *msg = _msg;

		struct chunk ck;
		unfill_ser_chunk(&msg->dat, &ck);

		hdarr_set(&sim->w->chunks.hd, &ck.pos, &ck);

		sim->changed.chunks = true;
		break;
	}
	case mt_action:
	{

		struct msg_action *msg = _msg;

		switch (msg->mt) {
		case amt_add:
			break;
		case amt_del:
			sim_remove_action(sim, msg->id);
			break;
		default:
			assert(false);
			break;
		}

		break;
	}
	default:
		assert(false);
	}
}
