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
	hdarr_init(&w->ents, 2048, sizeof(ent_id_t), sizeof(struct ent), world_ent_key_getter);

	darr_init(&w->graveyard, sizeof(ent_id_t));
	darr_init(&w->spawn, sizeof(struct ent));

	w->seq = 1;
}

void
world_reset(struct world *w)
{
	hdarr_clear(&w->chunks.hd);
	hdarr_clear(&w->ents);
	darr_clear(&w->graveyard);
	darr_clear(&w->spawn);
	w->seq = 1;
}

void
world_despawn(struct world *w, uint32_t id)
{
	hdarr_del(&w->ents, &id);
}

bool
world_load(struct world *w, struct world_loader *wl)
{
	if (!wl->loader) {
		LOG_W(log_misc, "no world loader provided");
		return false;
	}

	return wl->loader(w, wl->opts);
}
