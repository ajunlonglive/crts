#ifndef SERVER_AGGREGATE_MSGS_H
#define SERVER_AGGREGATE_MSGS_H

#include "server/sim/sim.h"
#include "shared/net/net_ctx.h"

void aggregate_msgs(struct simulation *sim, struct net_ctx *nx);
#endif