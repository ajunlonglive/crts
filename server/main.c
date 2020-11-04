#include "posix.h"

#include <string.h>

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/net.h"
#include "server/opts.h"
#include "server/sim/sim.h"
#include "shared/net/net_ctx.h"
#include "shared/pathfind/preprocess.h"
#include "shared/serialize/to_disk.h"
#include "shared/sim/action.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"
#include "shared/util/time.h"
#include "tracy.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const*argv)
{
	log_init();
	LOG_I("initializing server");

	struct timespec tick_st;

	struct server_opts so;

	process_s_opts(argc, argv, &so);

	struct world *w = world_init();

	if (so.world) {
		if (!load_world_from_path(so.world, &w->chunks)) {
			return 1;
		}
	} else {
		LOG_I("generating world");
		struct terragen_ctx ctx = { 0 };
		terragen_init(&ctx, so.tg_opts);
		terragen(&ctx, &w->chunks);
	}


	uint32_t i;
	for (i = 0; i < hdarr_len(w->chunks.hd); ++i) {
		ag_preprocess_chunk(&w->chunks, hdarr_get_by_i(w->chunks.hd, i));
	}

	struct simulation *sim = sim_init(w);
	struct net_ctx *nx = net_init(sim);

	long slept_ns = 0;

	handle_msgs_init();

	LOG_I("server initialized");
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	while (1) {
		TracyCFrameMark;

		recv_msgs(nx);

		simulate(sim);

		aggregate_msgs(sim, nx);

		send_msgs(nx);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	return 0;
}
