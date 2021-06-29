#include "posix.h"

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/server.h"
#include "server/sim/ai.h"
#include "shared/pathfind/preprocess.h"
#include "shared/util/log.h"
#include "tracy.h"

bool
init_server(struct server *s, struct world_loader *wl,
	struct server_opts *opts)
{
	*s = (struct server) { 0 };

	LOG_I(log_misc, "initializing server");

	world_init(&s->w);

	if (!world_load(&s->w, wl)) {
		return false;
	}

	ag_init_components(&s->w.chunks);

	sim_init(&s->w, &s->sim);

	msgr_init(&s->msgr, &s->sim, server_handle_msg, 0);

	LOG_I(log_misc, "server initialized");

	return true;
}

void
server_tick(struct server *s, uint32_t ticks)
{
	TracyCZoneAutoS;

	msgr_recv(&s->msgr);

	if (!s->sim.paused) {
		ai_tick(&s->sim);

		uint32_t i;
		for (i = 0; i < ticks; ++i) {
			simulate(&s->sim);
			// TODO performance: we should only need to do this once after
			// N ticks, but right now the simulate does cleanup that will
			// remove events before they are caught, e.g. emt_kill
			aggregate_msgs(&s->sim, &s->msgr);
		}
	}

	msgr_queue(&s->msgr, mt_server_info, &(struct msg_server_info) {
		.fps = 1.0f / s->prof.server_tick.avg,
	}, 0, priority_dont_resend);

	msgr_send(&s->msgr);

	TracyCZoneAutoE;
}
