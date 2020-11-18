#include "posix.h"

#include <string.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/local.h"
#include "shared/pathfind/macros.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/tiles.h"
#include "shared/util/log.h"
#include "tracy.h"

/*
   _0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 224, 240,
   _1,  17,  33,  49,  65,  81,  97, 113, 129, 145, 161, 177, 193, 209, 225, 241,
   _2,  18,  34,  50,  66,  82,  98, 114, 130, 146, 162, 178, 194, 210, 226, 242,
   _3,  19,  35,  51,  67,  83,  99, 115, 131, 147, 163, 179, 195, 211, 227, 243,
   _4,  20,  36,  52,  68,  84, 100, 116, 132, 148, 164, 180, 196, 212, 228, 244,
   _5,  21,  37,  53,  69,  85, 101, 117, 133, 149, 165, 181, 197, 213, 229, 245,
   _6,  22,  38,  54,  70,  86, 102, 118, 134, 150, 166, 182, 198, 214, 230, 246,
   _7,  23,  39,  55,  71,  87, 103, 119, 135, 151, 167, 183, 199, 215, 231, 247,
   _8,  24,  40,  56,  72,  88, 104, 120, 136, 152, 168, 184, 200, 216, 232, 248,
   _9,  25,  41,  57,  73,  89, 105, 121, 137, 153, 169, 185, 201, 217, 233, 249,
   10,  26,  42,  58,  74,  90, 106, 122, 138, 154, 170, 186, 202, 218, 234, 250,
   11,  27,  43,  59,  75,  91, 107, 123, 139, 155, 171, 187, 203, 219, 235, 251,
   12,  28,  44,  60,  76,  92, 108, 124, 140, 156, 172, 188, 204, 220, 236, 252,
   13,  29,  45,  61,  77,  93, 109, 125, 141, 157, 173, 189, 205, 221, 237, 253,
   14,  30,  46,  62,  78,  94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 254,
   15,  31,  47,  63,  79,  95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255,

   48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
   _0,                                                                        32,
   _1,                                                                        33,
   _2,                                                                        34,
   _3,                                                                        35,
   _4,                                                                        36,
   _5,                                                                        37,
   _6,                                                                        38,
   _7,                                                                        39,
   _8,                                                                        40,
   _9,                                                                        41,
   10,                                                                        42,
   11,                                                                        43,
   12,                                                                        44,
   13,                                                                        45,
   14,                                                                        46,
   15,                                                                        47,
   16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
 */

/* format: index, index within adjacent chunk
 * index format: 0: x-col, 2: x-col
 */
const uint32_t ag_component_node_indices[CHUNK_PERIM][2] = {
	/* left */
	{ 0,  240, }, { 1,  241, }, { 2,  242, }, { 3,  243, },
	{ 4,  244, }, { 5,  245, }, { 6,  246, }, { 7,  247, },
	{ 8,  248, }, { 9,  249, }, { 10, 250, }, { 11, 251, },
	{ 12, 252, }, { 13, 253, }, { 14, 254, }, { 15, 255, },

	/* bottom */
	{ 15,  0,   }, { 31,  16,  }, { 47,  32,  }, { 63,  48,  },
	{ 79,  64,  }, { 95,  80,  }, { 111, 96,  }, { 127, 112, },
	{ 143, 128, }, { 159, 144, }, { 175, 160, }, { 191, 176, },
	{ 207, 192, }, { 223, 208, }, { 239, 224, }, { 255, 240, },

	/* right */
	{ 240,  0, }, { 241,  1, }, { 242,  2, }, { 243,  3, },
	{ 244,  4, }, { 245,  5, }, { 246,  6, }, { 247,  7, },
	{ 248,  8, }, { 249,  9, }, { 250, 10, }, { 251, 11, },
	{ 252, 12, }, { 253, 13, }, { 254, 14, }, { 255, 15, },

	/* top */
	{ 0,   15,  }, { 16,  31,  }, { 32,  47,  }, { 48,  63,  },
	{ 64,  79,  }, { 80,  95,  }, { 96,  111, }, { 112, 127, },
	{ 128, 143, }, { 144, 159, }, { 160, 175, }, { 176, 191, },
	{ 192, 207, }, { 208, 223, }, { 224, 239, }, { 240, 255, },
};

static uint8_t throwaway[MAXPATH_LOCAL];

static void
fill_trav_mat(const struct chunk *ck, enum trav_type tt, uint8_t *trav)
{
	uint32_t i;

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		SB1_SET(trav, i, tile_is_traversable(((uint8_t *)ck->tiles)[i], tt));

		assert(SB1_GET(trav, i) == tile_is_traversable(((uint8_t *)ck->tiles)[i], tt));
	}
}

static void
fill_adj_mat(uint8_t *trav, uint8_t *adj)
{
	uint32_t i;
	uint8_t v;

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		v = TRAV_LEFT_OF(trav, i)
		    | (TRAV_BELOW(trav, i) << 1)
		    | (TRAV_RIGHT_OF(trav, i) << 2)
		    | (TRAV_ABOVE(trav, i) << 3);

		SB4_SET(adj, i, v);

		assert(v == SB4_GET(adj, i));
	}
}

static void
find_regions(struct ag_component *agc, uint8_t *trav)
{
	uint16_t j, i, region = 1, cenx, ceny, region_size;
	uint8_t checked[32] = { 0 }, open_set[256] = { 0 }, open_set_len = 0;
	uint8_t zchecked[32] = { 0 };

	for (j = 0; j < CHUNK_SIZE * CHUNK_SIZE; ++j) {
		if (SB1_GET(checked, j)) {
			continue;
		} else if (!SB1_GET(trav, j)) {
			SB4_SET(agc->region_map, j, NULL_REGION);
			SB1_SET(checked, j, 1);
			continue;
		}

		i = j;
		cenx = ceny = region_size = 0;

		memset(zchecked, 0, 32);
		assert(open_set_len == 0);

		goto start_filling;

		while (open_set_len) {
			i = open_set[--open_set_len];

			if (SB1_GET(zchecked, i)) {
				continue;
			}
start_filling:
			assert(!SB1_GET(checked, i));
			SB1_SET(checked, i, 1);

			SB4_SET(agc->region_map, i, region);
			SB1_SET(zchecked, i, 1);
			assert(SB4_GET(agc->region_map, i) == region);

			cenx += i >> 4;
			ceny += i & 15;
			++region_size;

			if (TRAV_LEFT_OF(trav, i) && !SB1_GET(zchecked, LEFT_OF(i))) {
				open_set[open_set_len++] = LEFT_OF(i);
			}

			if (TRAV_RIGHT_OF(trav, i) && !SB1_GET(zchecked, RIGHT_OF(i))) {
				open_set[open_set_len++] = RIGHT_OF(i);
			}

			if (TRAV_ABOVE(trav, i) && !SB1_GET(zchecked, ABOVE(i))) {
				open_set[open_set_len++] = ABOVE(i);
			}

			if (TRAV_BELOW(trav, i) && !SB1_GET(zchecked, BELOW(i))) {
				open_set[open_set_len++] = BELOW(i);
			}
		}

		cenx /= region_size;
		ceny /= region_size;
		i = (cenx << 4) | ceny;

		if (SB4_GET(agc->region_map, i) == region) {
			agc->regions[region].center = i;
		} else {
			memset(zchecked, 0, 32);
			assert(open_set_len == 0);

			goto start_centroid;

			while (open_set_len) {
				i = open_set[--open_set_len];

				if (SB4_GET(agc->region_map, i) == region) {
					agc->regions[region].center = i;
					open_set_len = 0;
					break;
				} else if (SB1_GET(zchecked, i)) {
					continue;
				}
start_centroid:

				SB1_SET(zchecked, i, 1);

				if (HAS_LEFT_OF(i) && !SB1_GET(zchecked, LEFT_OF(i))) {
					open_set[open_set_len++] = LEFT_OF(i);
				}

				if (HAS_RIGHT_OF(i) && !SB1_GET(zchecked, RIGHT_OF(i))) {
					open_set[open_set_len++] = RIGHT_OF(i);
				}

				if (HAS_ABOVE(i) && !SB1_GET(zchecked, ABOVE(i))) {
					open_set[open_set_len++] = ABOVE(i);
				}

				if (HAS_BELOW(i) && !SB1_GET(zchecked, BELOW(i))) {
					open_set[open_set_len++] = BELOW(i);
				}
			}
		}

		assert(SB4_GET(agc->region_map, agc->regions[region].center) == region);

		++region;
		assert(region < MAX_REGIONS);
	}

	agc->regions_len = region;
}

static void
get_nbr_agcs(struct abstract_graph *ag, struct point cp, struct ag_component *nbr_agcs[4])
{
	cp.x -= CHUNK_SIZE;
	nbr_agcs[adjck_left] = hdarr_get(ag->components, &cp);

	cp.x += 2 * CHUNK_SIZE;
	nbr_agcs[adjck_right] = hdarr_get(ag->components, &cp);

	cp.x -= CHUNK_SIZE;
	cp.y -= CHUNK_SIZE;
	nbr_agcs[adjck_up] = hdarr_get(ag->components, &cp);

	cp.y += 2 * CHUNK_SIZE;
	nbr_agcs[adjck_down] = hdarr_get(ag->components, &cp);
}

void
ag_link_component(struct abstract_graph *ag, struct ag_component *agc)
{
	uint16_t s, k, i;
	uint8_t region, nbr_region, p, dd;
	struct ag_component *nbr_agcs[4] = { 0 }, *nbr_agc;

	get_nbr_agcs(ag, agc->pos, nbr_agcs);

	for (s = 0; s < 4; ++s) {
		if (!(nbr_agc = nbr_agcs[s])) {
			continue;
		}

		struct { uint16_t d; uint8_t i, s; } lattice[MAX_REGIONS][MAX_REGIONS] = { 0 };

		for (k = 0; k < CHUNK_SIZE; ++k) {
			i = s * CHUNK_SIZE + k;
			p = ag_component_node_indices[i][0];

			region = SB4_GET(agc->region_map, p);
			nbr_region = SB4_GET(nbr_agc->region_map, ag_component_node_indices[i][1]);

			if (region == NULL_REGION || nbr_region == NULL_REGION) {
				continue;
			}

			astar_local(nbr_agc, ag_component_node_indices[i][1],
				nbr_agc->regions[nbr_region].center, throwaway, &dd);

			if (lattice[region][nbr_region].s) {
				if (dd < lattice[region][nbr_region].d) {
					agc->regions[region].entrances[lattice[region][nbr_region].i] = p;
					lattice[region][nbr_region].d = dd;
				}
				continue;
			}

			lattice[region][nbr_region].i = agc->regions[region].edges_len;
			agc->regions[region].entrances[lattice[region][nbr_region].i] = p;
			lattice[region][nbr_region].d = dd;
			lattice[region][nbr_region].s = 1;

			agc->regions[region].edges[agc->regions[region].edges_len] = s | (nbr_region << 2);

			++agc->regions[region].edges_len;

			assert(agc->regions[region].edges_len < MAX_REGION_EDGES);
		}
	}
}

static void
set_component(const struct chunk *ck, struct ag_component *agc, enum trav_type tt)
{
	uint8_t trav[32] = { 0 };

	fill_trav_mat(ck, tt, trav);
	fill_adj_mat(trav, agc->adj_map);
	find_regions(agc, trav);
}

void
ag_reset_component(const struct chunk *ck, struct ag_component *agc,
	enum trav_type tt)
{
	*agc = (struct ag_component){ .pos = ck->pos };

	set_component(ck, agc, tt);
}

void
ag_init_components(struct chunks *cnks)
{
	uint32_t i;
	struct ag_component empty = { 0 }, *agc;
	struct chunk *ck;

	for (i = 0; i < hdarr_len(cnks->hd); ++i) {
		ck = hdarr_get_by_i(cnks->hd, i);
		empty.pos = ck->pos;
		hdarr_set(cnks->ag.components, &ck->pos, &empty);
		agc = hdarr_get(cnks->ag.components, &ck->pos);

		set_component(ck, agc, cnks->ag.trav);
	}

	for (i = 0; i < hdarr_len(cnks->ag.components); ++i) {
		agc = hdarr_get_by_i(cnks->ag.components, i);

		ag_link_component(&cnks->ag, agc);
	}
}

void
ag_print_component(struct ag_component *agc)
{
#ifndef NDEBUG
	uint16_t i;
	L("%d region(s)", agc->regions_len);

	uint8_t fk, r, e, j;
	for (i = 0; i < 256; ++i) {
		fk = (i / 16) + ((i % 16) * 16);
		r = SB4_GET(agc->region_map, fk);

		e = 0;
		for (j = 0; j < agc->regions[r].edges_len; ++j) {
			if (agc->regions[r].entrances[j] == fk) {
				e = 1;
				goto breakout;
			}
		}
breakout:

		if (e) {
			fprintf(stderr, "\033[4%dmE\033[0m", r);
		} else if (fk == agc->regions[r].center) {
			fprintf(stderr, "\033[4%dmC\033[0m", r);
		} else {
			fprintf(stderr, "%d", r);
		}

		if (!((i + 1) & 15)) {
			fputc('\n', stderr);
		}
	}
#endif
}
