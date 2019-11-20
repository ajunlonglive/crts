#include <stdlib.h>
#include "world.h"

#define ENT_STEP 10

struct world *world_init()
{
	struct world *w;

	w = malloc(sizeof(struct world));
	w->ecnt = 0;
	w->ecap = ENT_STEP;
	w->ents = calloc(ENT_STEP, sizeof(struct world));

	return w;
}

void world_spawn(struct world *w, struct ent *e)
{
	if (w->ecnt + 1 > w->ecap) {
		w->ecap += ENT_STEP;
		w->ents = realloc(w->ents, sizeof(struct world) * w->ecap);
	}

	w->ents[w->ecnt] = e;
	w->ecnt++;
}

struct ent *ent_init(int x, int y, char c)
{
	struct ent *e;

	e = malloc(sizeof(struct ent));
	e->x = x;
	e->y = y;
	e->c = c;

	return e;
}

