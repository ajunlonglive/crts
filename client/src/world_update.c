#include "log.h"
#include "world_update.h"

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
	e->alignment->max = eu->alignment;
}


void world_apply_update(struct world *w, struct update *ud)
{
	switch (ud->type) {
	case update_type_ent:
		world_apply_ent_update(w, ud->update);
		break;
	case update_type_chunk:

		break;
	case update_type_chunk_req:
	case update_type_poke:
	case update_type_action:
		break;
	}
}

