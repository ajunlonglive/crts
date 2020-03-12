#ifndef __WORLD_UPDATE_H
#define __WORLD_UPDATE_H

#include "client/sim.h"
#include "shared/messaging/server_message.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/world.h"

void world_update(struct simulation *sim, struct net_ctx *nx);
#endif
