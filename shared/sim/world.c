#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

struct world *
world_init(void)
{
	struct world *w;

	w = calloc(1, sizeof(struct world));
	chunks_init(&w->chunks);

	return w;
}

struct ent *
world_spawn(struct world *w)
{
	struct ent *e;
	size_t i = w->ents.len;
	union {
		void **vp;
		struct ent **ep;
	} ep = { .ep = &w->ents.e };

	get_mem(ep.vp, sizeof(struct ent), &w->ents.len, &w->ents.cap);

	e = &w->ents.e[i];
	ent_init(e);
	e->id = i;

	return e;
}

void
world_despawn(struct world *w, size_t index)
{
	assert(0); //TODO this is broken, use memmove

	w->ents.len--;
	memcpy(&w->ents.e[index], &w->ents.e[w->ents.len], sizeof(struct ent));
	memset(&w->ents.e[w->ents.len], 0, sizeof(struct ent));
}
