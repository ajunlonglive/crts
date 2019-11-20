#include <stdlib.h>
#include "alignment.h"
#include "world.h"

#define ENT_STEP 10

struct world *world_init()
{
	struct world *w;

	w = malloc(sizeof(struct world));
	w->ecnt = 0;
	w->ecap = ENT_STEP;
	w->ents = calloc(ENT_STEP, sizeof(struct ent));

	return w;
}

static void ent_init(struct ent *e)
{
	e = malloc(sizeof(struct ent));
	e->x = 0;
	e->y = 0;
	e->c = '?';

	e->satisfaction = 100;
	e->alignment = alignment_init();
	e->age = 0;
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

	return e;
}
