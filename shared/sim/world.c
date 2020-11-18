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


void
world_init(struct world *w)
{
	chunks_init(&w->chunks);
	hdarr_init(&w->ents, 512, sizeof(uint32_t), sizeof(struct ent), world_ent_key_getter);

#ifdef CRTS_SERVER
	darr_init(&w->graveyard, sizeof(uint32_t));
	darr_init(&w->spawn, sizeof(struct ent));
#endif

	w->seq = 1;
}

void
world_despawn(struct world *w, uint32_t id)
{
	hdarr_del(&w->ents, &id);
}
