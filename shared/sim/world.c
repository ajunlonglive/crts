#include "posix.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

static const void *
world_ent_key_getter(void *_e)
{
	struct ent *e = _e;

	return &e->id;
}

struct world *
world_init(void)
{
	struct world *w;

	w = calloc(1, sizeof(struct world));
	chunks_init(&w->chunks);
	w->ents = hdarr_init(512, sizeof(uint32_t), sizeof(struct ent), world_ent_key_getter);

#ifdef CRTS_SERVER
	w->graveyard = darr_init(sizeof(uint32_t));
	w->spawn = darr_init(sizeof(struct ent));
#endif

	w->seq = 1;

	return w;
}

struct ent *
world_spawn(struct world *w)
{
	struct ent e;

	ent_init(&e);
	e.id = w->seq++;

	hdarr_set(w->ents, &e.id, &e);

	return hdarr_get(w->ents, &e.id);
}

void
world_despawn(struct world *w, uint32_t id)
{
	hdarr_del(w->ents, &id);
}
