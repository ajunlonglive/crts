#ifndef SERVER_SIM_ENT_LOOKUP_H
#define SERVER_SIM_ENT_LOOKUP_H
#include <stdbool.h>
#include <stdint.h>

#include "server/sim/sim.h"

typedef bool ((*ent_lookup_pred)(struct ent *e, void *ctx));
typedef void ((*ent_lookup_cb)(struct ent *e, void *ctx));

uint16_t ent_lookup(struct simulation *sim, struct pgraph *pg, void *usr_ctx,
	ent_lookup_pred pred, ent_lookup_cb cb, uint16_t needed,
	const struct point *origin);
#endif
