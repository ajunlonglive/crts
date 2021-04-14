#include "posix.h"

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/server.h"
#include "shared/pathfind/preprocess.h"
#include "shared/util/log.h"
#include "tracy.h"

bool
init_server(struct server *s, struct world_loader *wl,
	struct server_opts *opts)
{
	*s = (struct server) { 0 };

	LOG_I("initializing server");

	world_init(&s->w);

	if (!world_load(&s->w, wl)) {
		return false;
	}

	ag_init_components(&s->w.chunks);

	sim_init(&s->w, &s->sim);

	msgr_init(&s->msgr, &s->sim, server_handle_msg, 0);

	LOG_I("server initialized");

	return true;
}

void
server_tick(struct server *s, uint32_t ticks)
{
	TracyCZoneAutoS;

	msgr_recv(&s->msgr);

	uint32_t i;
	for (i = 0; i < ticks; ++i) {
		simulate(&s->sim);
	}

	aggregate_msgs(&s->sim, &s->msgr);

	msgr_send(&s->msgr);

	TracyCZoneAutoE;
}
