#include "posix.h"

#include "shared/math/geom.h"
#include "shared/sim/ent.h"
#include "shared/sim/ent_buckets.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "tracy.h"

#define BUCKET_SIZE 64 - 2

enum bucket_flags {
	bucket_flag_have_next = 1 << 0,
};

struct bucket {
	uint32_t e[BUCKET_SIZE];
	uint32_t next;
	uint8_t len;
	uint8_t flags;
};

_Static_assert(sizeof(struct bucket) == 4 * 64, "");

void
ent_buckets_set(struct ent_buckets *eb, uint32_t e_id, struct point *p)
{
	TracyCZoneAutoS;
	const uint64_t *bucket_id;
	struct bucket *b;

	/* LOG_W(log_misc, "setting (%d, %d) %d", p->x, p->y, e_id); */

	if ((bucket_id = hash_get(&eb->keys, p))) {
		b = darr_get(&eb->buckets, *bucket_id);
		/* LOG_W(log_misc, "found bucket len:%d, flg:%d, next: %d", b->len, b->flags, b->next); */

		while (b->len >= BUCKET_SIZE) {
			/* LOG_W(log_misc, "full, finding next"); */
			if (!(b->flags & bucket_flag_have_next)) {
				/* LOG_W(log_misc, "no next, making one"); */
				uint64_t new_id = darr_push(&eb->buckets, &(struct bucket) { .e = { e_id }, .len = 1 });
				b = darr_get(&eb->buckets, *bucket_id);
				b->flags |= bucket_flag_have_next;
				b->next = new_id;
				TracyCZoneAutoE;
				return;
			}
			b = darr_get(&eb->buckets, b->next);
			/* LOG_W(log_misc, "found next bucket len:%d, flg:%d", b->len, b->flags); */
		}

		/* LOG_W(log_misc, "setting ent in bucket"); */
		b->e[b->len] = e_id;
		++b->len;
	} else {
		/* LOG_W(log_misc, "no bucket found, setting ent in new bucket"); */
		hash_set(&eb->keys, p, darr_push(&eb->buckets, &(struct bucket) { .e = { e_id }, .len = 1 }));
	}
	TracyCZoneAutoE;
}

void
ent_buckets_unset(struct ent_buckets *eb, uint32_t e_id, struct point *p)
{
	struct bucket *b;
	uint8_t i;

	TracyCZoneAutoS;
	/* LOG_W(log_misc, "unsetting (%d, %d) %d", p->x, p->y, e_id); */

	/* assert(hash_get(&eb->keys, p)); */

	b = darr_get(&eb->buckets, *hash_get(&eb->keys, p));
	/* LOG_W(log_misc, "found bucket len:%d, flg:%d, next: %d", b->len, b->flags, b->next); */

	uint32_t d = 0;

	while (true) {
		for (i = 0; i < b->len; ++i) {
			if (b->e[i] == e_id) {
				if (--b->len) {
					b->e[i] = b->e[b->len];
				}
				TracyCZoneAutoE;
				return;
			}
		}
		if (!(b->flags & bucket_flag_have_next)) {
			LOG_W(log_misc, "d: %d, ent not found in bucket len:%d, flg:%d, next: %d", d, b->len, b->flags, b->next);
		}
		assert((b->flags & bucket_flag_have_next) && "ent not found in bucket");
		b = darr_get(&eb->buckets, b->next);
		++d;
		/* LOG_W(log_misc, "found next bucket len:%d, flg:%d", b->len, b->flags); */
	}
}

void
for_each_ent_at(struct ent_buckets *eb, struct hdarr *ents, const struct point *p, void *ctx, for_each_ent_at_cb func)
{
	const uint64_t *bucket_id;
	struct bucket *b;
	uint8_t i;

	TracyCZoneAutoS;

	if ((bucket_id = hash_get(&eb->keys, p))) {
		b = darr_get(&eb->buckets, *bucket_id);

		while (true) {
			for (i = 0; i < b->len; ++i) {
				if (func(ctx, hdarr_get_by_i(ents, b->e[i])) != ir_cont) {
					TracyCZoneAutoE;
					return;
				}

			}

			if (!(b->flags & bucket_flag_have_next)) {
				TracyCZoneAutoE;
				return;
			}
			b = darr_get(&eb->buckets, b->next);
		}
	}

	TracyCZoneAutoE;
}

void
make_ent_buckets(struct ent_buckets *eb, struct hdarr *ents)
{
	uint32_t i;
	struct ent *e;
	const uint64_t *bucket_id;
	struct bucket *b;

	TracyCZoneAutoS;

	darr_clear(&eb->buckets);
	hash_clear(&eb->keys);

	for (i = 0; i < ents->darr.len; ++i) {
		e = hdarr_get_by_i(ents, i);

		/* LOG_W(log_misc, "setting (%d, %d) %d", p->x, p->y, e_id); */

		if ((bucket_id = hash_get(&eb->keys, &e->pos))) {
			b = darr_get(&eb->buckets, *bucket_id);
			/* LOG_W(log_misc, "found bucket len:%d, flg:%d, next: %d", b->len, b->flags, b->next); */

			while (b->len >= BUCKET_SIZE) {
				/* LOG_W(log_misc, "full, finding next"); */
				if (!(b->flags & bucket_flag_have_next)) {
					/* LOG_W(log_misc, "no next, making one"); */
					uint64_t new_id = darr_push(&eb->buckets, &(struct bucket) { .e = { i }, .len = 1 });
					b = darr_get(&eb->buckets, *bucket_id);
					b->flags |= bucket_flag_have_next;
					b->next = new_id;
					goto cont;
				}
				b = darr_get(&eb->buckets, b->next);
				/* LOG_W(log_misc, "found next bucket len:%d, flg:%d", b->len, b->flags); */
			}

			/* LOG_W(log_misc, "setting ent in bucket"); */
			b->e[b->len] = i;
			++b->len;
		} else {
			/* LOG_W(log_misc, "no bucket found, setting ent in new bucket"); */
			hash_set(&eb->keys, &e->pos, darr_push(&eb->buckets, &(struct bucket) { .e = { i }, .len = 1 }));
		}

cont:
		{};
	}

	TracyCZoneAutoE;
}

void
ent_buckets_init(struct ent_buckets *eb)
{
	darr_init(&eb->buckets, sizeof(struct bucket));
	hash_init(&eb->keys, 2048, sizeof(struct point));
}
