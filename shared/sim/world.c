#include <string.h>
#include <stdlib.h>
#include "sim/alignment.h"
#include "sim/ent.h"
#include "sim/world.h"
#include "util/log.h"

struct world *world_init(void)
{
	struct world *w;

	w = malloc(sizeof(struct world));
	w->ecnt = 0;
	w->ecap = ENT_STEP;
	w->ents = calloc(ENT_STEP, sizeof(struct ent));
	w->chunks = hash_init(sizeof(struct point));

	return w;
}

struct ent *world_spawn(struct world *w)
{
	struct ent *e;

	if (w->ecnt + 1 > w->ecap) {
		w->ecap += ENT_STEP;
		w->ents = realloc(w->ents, sizeof(struct ent) * w->ecap);
	}

	w->ecnt++;
	e = &w->ents[w->ecnt - 1];
	ent_init(e);
	e->id = w->ecnt - 1;

	return e;
}

void world_despawn(struct world *w, int i)
{
	w->ecnt--;
	memcpy(&w->ents[i], &w->ents[w->ecnt], sizeof(struct ent));
	memset(&w->ents[w->ecnt], 0, sizeof(struct ent));
}
