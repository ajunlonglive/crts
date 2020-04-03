#include "server/sim/ent_buckets.h"
#include "shared/math/geom.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

static enum iteration_result
count_bucket_size(void *_h, void *_e)
{
	struct ent *e = _e;
	struct hash *h = _h;
	struct point p;
	const size_t *st;

	p = point_mod(&e->pos, BUCKET_SIZE);

	if ((st = hash_get(h, &p))) {
		hash_set(h, &p, *st + 1);
	} else {
		hash_set(h, &p, 1);
	}

	return ir_cont;
}

static enum iteration_result
calc_offsets(void *_eb, void *p, size_t amnt)
{
	struct ent_buckets *eb = _eb;

	hash_set(eb->keys, p, eb->total);
	hash_set(eb->counts, p, 0);
	eb->total += amnt;

	return ir_cont;
}

static enum iteration_result
put_in_buckets(void *_eb, void *_e)
{
	struct ent_buckets *eb = _eb;
	struct ent *e = _e;
	struct point p;
	size_t off, cnt;

	p = point_mod(&e->pos, BUCKET_SIZE);

	cnt = *hash_get(eb->counts, &p);
	off = *hash_get(eb->keys, &p);

	darr_set(eb->buckets, cnt + off, &e->id);
	hash_set(eb->counts, &p, cnt + 1);

	return ir_cont;
}

void
for_each_ent_at(struct ent_buckets *eb, struct hdarr *ents, const struct point *p,
	void *ctx, enum iteration_result ((*func)(void *ctx, struct ent *e)))
{
	const size_t *off, *cnt;
	size_t i;
	struct point q = point_mod(p, BUCKET_SIZE);
	struct ent *e;

	if ((off = hash_get(eb->keys, &q)) && (cnt = hash_get(eb->counts, &q))) {
		for (i = *off; i < (*off + *cnt); ++i) {
			if ((e = hdarr_get(ents, darr_get(eb->buckets, i)))
			    && points_equal(&e->pos, p)) {
				if (func(ctx, e) != ir_cont) {
					return;
				}
			}
		}
	}
}

void
make_ent_buckets(struct hdarr *ents, struct ent_buckets *eb)
{
	hdarr_for_each(ents, eb->counts, count_bucket_size);

	hash_for_each_with_keys(eb->counts, eb, calc_offsets);

	darr_grow_to(eb->buckets, hdarr_len(ents));

	hdarr_for_each(ents, eb, put_in_buckets);
}

void
ent_buckets_init(struct ent_buckets *eb)
{
	eb->buckets = darr_init(sizeof(struct ent *));
	eb->keys = hash_init(2048, 1, sizeof(struct point));
	eb->counts = hash_init(2048, 1, sizeof(struct point));
	eb->total = 0;
}

void
ent_buckets_clear(struct ent_buckets *eb)
{
	/* darr_clear(eb->buckets); // doesn't accomplish anything */
	hash_clear(eb->keys);
	hash_clear(eb->counts);
	eb->total = 0;
}
