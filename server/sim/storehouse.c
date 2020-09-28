#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <assert.h>

#include "server/sim/ent.h"
#include "server/sim/storehouse.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

bool
storehouse_take(struct storehouse_storage *st, uint32_t type)
{
	uint32_t i;

	for (i = 0; i < STOREHOUSE_SLOTS; ++i) {
		if (st->type[i] == type && st->amnt[i]) {
			--st->amnt[i];
			return true;
		}
	}

	return false;
}

bool
storehouse_store(struct storehouse_storage *st, uint32_t type)
{
	uint32_t i;

	for (i = 0; i < STOREHOUSE_SLOTS; ++i) {
		if (!st->type[i]) {
			st->type[i] = type;
			st->amnt[i] = 1;
			return true;
		} else if (st->type[i] == type && st->amnt[i] < 255) {
			++st->amnt[i];
			return true;
		}
	}

	return false;
}

bool
storehouse_contains(struct storehouse_storage *st, uint32_t type)
{
	uint32_t i;

	for (i = 0; i < STOREHOUSE_SLOTS; ++i) {
		if (st->type[i] == type && st->amnt[i]) {
			return true;
		}
	}

	return false;
}

bool
storehouse_can_hold(struct storehouse_storage *st, uint32_t type)
{
	uint32_t i;

	for (i = 0; i < STOREHOUSE_SLOTS; ++i) {
		if (!st->type[i] || (st->type[i] == type && st->amnt[i] < 255)) {
			return true;
		}
	}

	return false;
}

struct storehouse_storage *
nearest_storehouse(struct chunks *cnks, const struct point *p, uint32_t type)
{
	uint32_t mindist = UINT32_MAX, i, dist;
	struct storehouse_storage *st, *minst = NULL;

	for (i = 0; i < darr_len(cnks->storehouses); ++i) {
		st = darr_get(cnks->storehouses, i);

		if (type && storehouse_can_hold(st, type)) {
		}

		if ((dist = square_dist(p, &st->pos)) < mindist) {
			mindist = dist;
			minst = st;
		}
	}

	return minst;
}

static struct storehouse_storage *
try_get_storehouse_storage_at(struct chunks *cnks, const struct point *p)
{
	uint32_t i;
	struct storehouse_storage *st;

	for (i = 0; i < darr_len(cnks->storehouses); ++i) {
		st = darr_get(cnks->storehouses, i);
		if (points_equal(&st->pos, p)) {
			return st;
		}
	}

	return NULL;
}

struct storehouse_storage *
get_storehouse_storage_at(struct chunks *cnks, const struct point *p)
{
	struct storehouse_storage *st = try_get_storehouse_storage_at(cnks, p);
	assert(st);
	return st;
}

void
create_storehouse(struct world *w, const struct point *p, uint16_t owner)
{
	assert(!try_get_storehouse_storage_at(&w->chunks, p));
	struct storehouse_storage st = { .pos = *p, .owner = owner };

	struct ent *e = spawn_ent(w);
	e->pos = *p;
	e->type = et_storehouse;
	st.ent = e->id;

	darr_push(w->chunks.storehouses, &st);
}

void
destroy_storehouse(struct world *w, const struct point *p)
{
	struct storehouse_storage *st =
		get_storehouse_storage_at(&w->chunks, p);

	uint32_t i, j;
	struct ent *drop;

	for (i = 0; i < STOREHOUSE_SLOTS; ++i) {
		for (j = 0; j < st->amnt[i]; ++j) {
			drop = spawn_ent(w);
			drop->pos = *p;
			drop->type = st->type[i];
		}
	}

	struct ent *e = hdarr_get(w->ents, &st->ent);
	assert(e);
	destroy_ent(w, e);

	for (i = 0; i < darr_len(w->chunks.storehouses); ++i) {
		st = darr_get(w->chunks.storehouses, i);
		if (points_equal(&st->pos, p)) {
			break;
		}
	}

	assert(i < darr_len(w->chunks.storehouses));

	darr_del(w->chunks.storehouses, i);
}
