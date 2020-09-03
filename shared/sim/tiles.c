#include "posix.h"

#include "shared/constants/globals.h"
#include "shared/sim/tiles.h"
#include "shared/types/hash.h"

/* bool */
/* find_tile(enum tile t, struct chunks *cnks, const struct rectangle *rect, */
/* 	const struct point *start, struct point *p, struct hash *skip) */
/* { */
/* 	struct point q, r, c = { 0, 0 }; */
/* 	uint32_t dist, cdist = UINT32_MAX; */
/* 	bool found = false; */

/* 	for (p->x = rect->pos.x; p->x < rect->pos.x + (int64_t)rect->width; ++p->x) { */
/* 		for (p->y = rect->pos.y; p->y < rect->pos.y + (int64_t)rect->height; ++p->y) { */
/* 			q = nearest_chunk(p); */
/* 			r = point_sub(p, &q); */

/* 			if (get_chunk(cnks, &q)->tiles[r.x][r.y] == t) { */
/* 				if (skip != NULL && hash_get(skip, p) != NULL) { */
/* 					continue; */
/* 				} */

/* 				found = true; */
/* 				dist = square_dist(start, p); */
/* 				if (dist < cdist) { */
/* 					cdist = dist; */
/* 					c = *p; */
/* 				} */
/* 			} */
/* 		} */
/* 	} */

/* 	*p = c; */
/* 	return found; */
/* } */

enum tile
get_tile_at(struct chunks *cnks, const struct point *p)
{
	struct chunk *ck = get_chunk_at(cnks, p);
	struct point rp = point_sub(p, &ck->pos);

	return ck->tiles[rp.x][rp.y];
}

bool
find_adj_tile(struct chunks *cnks, struct point *s, struct point *rp,
	struct rectangle *r, enum tile t, uint8_t et, uint8_t reject[4],
	bool (*pred)(enum tile t, uint8_t et))
{
	enum tile tt;
	struct point p[4] = {
		{ s->x + 1, s->y     },
		{ s->x - 1, s->y     },
		{ s->x,     s->y + 1 },
		{ s->x,     s->y - 1 },
	};
	size_t i;

	for (i = 0; i < 4; ++i) {
		if (r && !point_in_rect(&p[i], r)) {
			continue;
		} else if (reject && reject[i]) {
			continue;
		}

		tt = get_tile_at(cnks, &p[i]);

		if (tt == t || (pred && pred(tt, et))) {
			if (rp) {
				*rp = p[i];
			}
			return true;
		}
	}

	return false;
}

bool
tile_is_traversable(enum tile t, uint8_t trav)
{
	return gcfg.tiles[t].trav_type & trav;
}

bool
is_traversable(struct chunks *cnks, const struct point *p, uint8_t t)
{
	return tile_is_traversable(get_tile_at(cnks, p), t);
}

