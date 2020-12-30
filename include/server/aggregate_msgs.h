#ifndef SERVER_AGGREGATE_MSGS_H
#define SERVER_AGGREGATE_MSGS_H

#include "server/sim/sim.h"
#include "shared/msgr/msgr.h"
/* #include "shared/net/net_ctx.h" */

struct package_ent_updates_ctx {
	struct msgr *msgr;
	msg_addr_t dest;
	bool all_alive;
};

enum iteration_result check_ent_updates(void *_ctx, void *_e);
void aggregate_msgs(struct simulation *sim, struct msgr *msgr);
#endif
