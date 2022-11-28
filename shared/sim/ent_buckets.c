#include "posix.h"

#include "shared/math/geom.h"
#include "shared/sim/ent.h"
#include "shared/sim/ent_buckets.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "tracy.h"

static void
for_each_ent_at(struct hash *eb, struct point3d *p, void *ctx, for_each_ent_at_cb func)
{
	uint64_t *ptr;

	/* L(log_cli, "checking %d, %d, %d", p->x, p->y, p->z); */

	if (!(ptr = hash_get(eb, p))) {
		return;
	}

	func(ctx, (struct ent *)(*ptr));
}

void
for_each_ent_adjacent_to(struct hash *eb, const struct ent *e, void *ctx, for_each_ent_at_cb func)
{
	struct point3d p = { .x = e->pos.x, .y = e->pos.y, .z = e->z };

	/* L(log_cli, "center %d, %d, %d", p.x, p.y, p.z); */
	TracyCZoneAutoS;

	++p.x;
	for_each_ent_at(eb, &p, ctx, func);

	p.x -= 2;
	for_each_ent_at(eb, &p, ctx, func);

	++p.x;
	++p.y;
	for_each_ent_at(eb, &p, ctx, func);

	p.y -= 2;
	for_each_ent_at(eb, &p, ctx, func);

	++p.y;

	--p.z;
	for_each_ent_at(eb, &p, ctx, func);

	p.z += 2;
	for_each_ent_at(eb, &p, ctx, func);

	TracyCZoneAutoE;
}

void
make_ent_buckets(struct hash *eb, struct hdarr *ents)
{
	uint32_t i;
	struct ent *e;

	TracyCZoneAutoS;

	hash_clear(eb);

	for (i = 0; i < ents->darr.len; ++i) {
		e = hdarr_get_by_i(ents, i);

		struct point3d key = { e->pos.x, e->pos.y, e->z, };

		/* LOG_W(log_misc, "setting (%d, %d) %d", p->x, p->y, e_id); */

		while (hash_get(eb, &key)) {
			++e->z;
			++key.z;
			e->state |= es_modified;
			e->modified |= eu_pos;
		}

		hash_set(eb, &key, e->id);
		/* LOG_W(log_misc, "found bucket len:%d, flg:%d, next: %d", b->len, b->flags, b->next); */
	}
	TracyCZoneAutoE;
}

void
ent_buckets_init(struct hash *eb)
{
	hash_init(eb, 2048, sizeof(struct point3d));
}
