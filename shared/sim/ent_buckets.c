#include "posix.h"

#include "shared/math/geom.h"
#include "shared/sim/ent.h"
#include "shared/sim/ent_buckets.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "tracy.h"

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

	hash_set(&eb->keys, p, eb->total);
	hash_set(&eb->counts, p, 0);
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

	cnt = *hash_get(&eb->counts, &p);
	off = *hash_get(&eb->keys, &p);

	darr_set(&eb->buckets, cnt + off, &e);
	hash_set(&eb->counts, &p, cnt + 1);

	return ir_cont;
}

struct for_each_bucket_proxy_ctx {
	for_each_bucket_cb cb;
	void *usr_ctx;
};

static enum iteration_result
for_each_bucket_proxy(void *_ctx, void *_p, size_t _)
{
	struct for_each_bucket_proxy_ctx *ctx = _ctx;
	const struct point *p = _p;

	return ctx->cb(ctx->usr_ctx, p);
}

void
for_each_bucket(struct ent_buckets *eb, void *usr_ctx, for_each_bucket_cb cb)
{
	struct for_each_bucket_proxy_ctx ctx = {
		.cb = cb,
		.usr_ctx = usr_ctx,
	};
	hash_for_each_with_keys(&eb->keys, &ctx, for_each_bucket_proxy);
}

bool
for_each_ent_in_bucket(struct ent_buckets *eb, struct hdarr *ents, const struct point *b,
	void *ctx, for_each_ent_at_cb cb)
{
	const size_t *off, *cnt;
	size_t i;
	struct ent **e;

	if ((off = hash_get(&eb->keys, b)) && (cnt = hash_get(&eb->counts, b))) {
		for (i = *off; i < (*off + *cnt); ++i) {
			e = darr_get(&eb->buckets, i);

			if (cb(ctx, *e) != ir_cont) {
				return true;
			}
		}
	}

	return false;
}


void
for_each_ent_at(struct ent_buckets *eb, struct hdarr *ents, const struct point *p,
	void *ctx, for_each_ent_at_cb func)
{
	const size_t *off, *cnt;
	size_t i;
	struct point q = point_mod(p, BUCKET_SIZE);
	struct ent **e;

	if ((off = hash_get(&eb->keys, &q)) && (cnt = hash_get(&eb->counts, &q))) {
		for (i = *off; i < (*off + *cnt); ++i) {
			if ((e = darr_get(&eb->buckets, i))
			    && points_equal(&(*e)->pos, p)) {
				if (func(ctx, *e) != ir_cont) {
					return;
				}
			}
		}
	}
}

void
make_ent_buckets(struct hdarr *ents, struct ent_buckets *eb)
{
	TracyCZoneAutoS;
	hdarr_for_each(ents, &eb->counts, count_bucket_size);

	hash_for_each_with_keys(&eb->counts, eb, calc_offsets);

	darr_grow_to(&eb->buckets, hdarr_len(ents));

	hdarr_for_each(ents, eb, put_in_buckets);
	TracyCZoneAutoE;
}

void
ent_buckets_init(struct ent_buckets *eb)
{
	darr_init(&eb->buckets, sizeof(struct ent *));
	hash_init(&eb->keys, 2048, sizeof(struct point));
	hash_init(&eb->counts, 2048, sizeof(struct point));
	eb->total = 0;
}

void
ent_buckets_clear(struct ent_buckets *eb)
{
	TracyCZoneAutoS;
	/* darr_clear(eb->buckets); // doesn't accomplish anything */
	hash_clear(&eb->keys);
	hash_clear(&eb->counts);
	eb->total = 0;
	TracyCZoneAutoE;
}
