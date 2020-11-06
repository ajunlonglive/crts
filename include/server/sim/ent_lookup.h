#ifndef SERVER_SIM_ENT_LOOKUP_H
#define SERVER_SIM_ENT_LOOKUP_H
#include <stdbool.h>
#include <stdint.h>

#include "server/sim/sim.h"
#include "shared/types/hash.h"
#include "shared/types/result.h"

typedef bool ((*ent_lookup_pred)(struct ent *e, void *ctx));
typedef void ((*ent_lookup_cb)(struct ent *e, void *ctx));

struct ent_lookup_ctx {
	const struct point *origin;
	void *usr_ctx;
	struct simulation *sim;
	struct darr *bucketheap;
	ent_lookup_pred pred;
	ent_lookup_cb cb;
	uint32_t checked;
	uint32_t total;
	uint16_t found;
	uint16_t needed;
	bool init;
};

bool ent_lookup(struct simulation *sim, struct ent_lookup_ctx *elctx);
void ent_lookup_reset(struct ent_lookup_ctx *elctx);
void ent_lookup_setup(struct ent_lookup_ctx *elctx);
void ent_lookup_teardown(struct ent_lookup_ctx *elctx);

uint32_t ent_count(struct hdarr *ents, void *ctx, ent_lookup_pred pred);
#endif
