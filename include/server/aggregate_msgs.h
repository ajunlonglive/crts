#ifndef SERVER_AGGREGATE_MSGS_H
#define SERVER_AGGREGATE_MSGS_H

#include "server/sim/sim.h"
#include "shared/net/net_ctx.h"

struct package_ent_updates_ctx {
	struct net_ctx *nx;
	cx_bits_t dest;
	bool all_alive;
};

enum iteration_result check_ent_updates(void *_ctx, void *_e);
void aggregate_msgs(struct simulation *sim, struct net_ctx *nx);
#endif
