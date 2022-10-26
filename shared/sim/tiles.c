#include "posix.h"

#include "shared/constants/globals.h"
#include "shared/sim/tiles.h"
#include "shared/types/hash.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

float
get_height_at(struct chunks *cnks, const struct point *p)
{
	struct point np = nearest_chunk(p);
	const struct chunk *ck;

	if ((ck = hdarr_get(&cnks->hd, &np))) {
		np = point_sub(p, &ck->pos);
		return ck->heights[np.x][np.y];
	} else {
		return 0.0f;
	}
}

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

const struct cfg_lookup_table ltbl_tiles = {
	"sea", tile_sea,
	"coast", tile_coast,
	"plain", tile_plain,
	"rock", tile_rock,
	"dirt", tile_dirt,
	"tree", tile_tree,
	"old_tree", tile_old_tree,
	"fire", tile_fire,
	"ash", tile_ash,
};
