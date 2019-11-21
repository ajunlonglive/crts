#include <string.h>
#include <stdlib.h>
#include "alignment.h"
#include "world.h"
#include "log.h"

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
	e->pos.x = 0;
	e->pos.y = 0;
	e->c = '?';

	e->satisfaction = 100;
	e->alignment = alignment_init();
	e->age = 0;
	e->task = 0;
	e->idle = 1;
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

	L("spawned int %p", e);

	return e;
}

void world_despawn(struct world *w, int i)
{
	w->ecnt--;
	memcpy(&w->ents[i], &w->ents[w->ecnt], sizeof(struct ent));
	memset(&w->ents[w->ecnt], 0, sizeof(struct ent));
}
