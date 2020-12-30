#include "posix.h"

#include "server/aggregate_msgs.h"
#include "server/api.h"
#include "server/handle_msg.h"
/* #include "server/net.h" */
#include "shared/pathfind/preprocess.h"
#include "shared/serialize/to_disk.h"
#include "shared/util/log.h"

bool
init_server(struct server *s, struct server_opts *so)
{
	*s = (struct server) { 0 };

	LOG_I("initializing server");

	world_init(&s->w);

	if (so->world) {
		if (!load_world_from_path(so->world, &s->w.chunks)) {
			return false;
		}
	} else {
		LOG_I("generating world");
		struct terragen_ctx ctx = { 0 };
		terragen_init(&ctx, so->tg_opts);
		terragen(&ctx, &s->w.chunks);
	}

	ag_init_components(&s->w.chunks);

	sim_init(&s->w, &s->sim);
	/* server_net_init(&s->sim, &s->nx); */
	msgr_init(&s->msgr, &s->sim, server_handle_msg, 0);

	LOG_I("server initialized");

	return true;
}

void
server_tick(struct server *s)
{
	msgr_recv(&s->msgr);

	simulate(&s->sim);

	aggregate_msgs(&s->sim, &s->msgr);

	msgr_send(&s->msgr);
}
