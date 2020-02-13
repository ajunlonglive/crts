#include <stdlib.h>
#include <string.h>

#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

struct world *
world_init(void)
{
	struct world *w;

	w = malloc(sizeof(struct world));
	w->ecnt = 0;
	w->ecap = ENT_STEP;
	w->ents = calloc(ENT_STEP, sizeof(struct ent));
	w->chunks = NULL;
	chunks_init(&w->chunks);

	return w;
}

struct ent *
world_spawn(struct world *w)
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

void
world_despawn(struct world *w, int i)
{
	w->ecnt--;
	memcpy(&w->ents[i], &w->ents[w->ecnt], sizeof(struct ent));
	memset(&w->ents[w->ecnt], 0, sizeof(struct ent));
}
