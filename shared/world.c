#include <string.h>
#include <stdlib.h>
#include "alignment.h"
#include "update.h"
#include "world.h"
#include "log.h"

#define ENT_STEP 100

struct world *world_init()
{
	struct world *w;

	w = malloc(sizeof(struct world));
	w->ecnt = 0;
	w->ecap = ENT_STEP;
	w->ents = calloc(ENT_STEP, sizeof(struct ent));

	return w;
}

void ent_init(struct ent *e)
{
	e->id = 0;
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
	e->id = w->ecnt - 1;

	return e;
}

static struct ent *find_or_create_ent(struct world *w, int id)
{
	size_t i;

	if (w->ecnt == 0 || (size_t)id > w->ecnt - 1) {
		if (w->ecap == 0 || (size_t)id > w->ecap - 1) {
			w->ecap = id + (ENT_STEP - id % ENT_STEP);
			L("reallocating ent pool to % d ", w->ecap);
			w->ents = realloc(w->ents, sizeof(struct ent) * w->ecap);
		}

		for (i = w->ecnt; i <= (size_t)id; i++) {
			ent_init(&w->ents[i]);
			w->ents[i].id = i;
		}

		w->ecnt = id + 1;
	}

	return &w->ents[id];
}

static void world_apply_ent_update(struct world * w, struct ent_update *eu)
{
	struct ent *e;

	e = find_or_create_ent(w, eu->id);
	e->pos = eu->pos;
}

void world_apply_update(struct world *w, struct update *ud)
{
	switch (ud->type) {
	case update_type_ent:
		world_apply_ent_update(w, ud->update);
		break;
	}

	update_destroy(ud);
}

void world_despawn(struct world *w, int i)
{
	w->ecnt--;
	memcpy(&w->ents[i], &w->ents[w->ecnt], sizeof(struct ent));
	memset(&w->ents[w->ecnt], 0, sizeof(struct ent));
}
