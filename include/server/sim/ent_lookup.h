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
	struct pgraph *pg;
	const struct point *origin;
	void *usr_ctx;
	struct hash *checked;
	ent_lookup_pred pred;
	ent_lookup_cb cb;
	uint16_t found;
	uint16_t needed;
	bool init;
	uint32_t radius;
};

enum result ent_lookup(struct simulation *sim, struct ent_lookup_ctx *elctx);
void ent_lookup_reset(struct ent_lookup_ctx *elctx);
void ent_lookup_setup(struct ent_lookup_ctx *elctx);
void ent_lookup_teardown(struct ent_lookup_ctx *elctx);
#endif
