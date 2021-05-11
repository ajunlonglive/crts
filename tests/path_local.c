#include "posix.h"

#include <stdlib.h>

#include "shared/pathfind/local.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

#define MAPD 1
#define MAPLEN ((MAPD * MAPD) * 256)
static char map[MAPLEN] = {
	"wwwwwwwwwwwwwwww" //-----------
	"w ww           w"
	"w ww w ww  www w"
	"w wwww ww  w w  "
	"  ww      ww www"
	"    www  www    "
	"      w ww  w   "
	"w      wwwww    "
	"ww              "
	"ww              "
	"ww              "
	"ww              "
	"ww              "
	"ww              "
	"ww              "
	"ww              "
};

static void
parse_map(struct chunks *cnks, uint8_t valid[], uint32_t *validi)
{
	uint32_t i;
	struct point p = { 0 }, cp, rp;
	struct chunk *ck;
	enum tile t;

	for (i = 0; i < MAPLEN; ++i) {
		p.x = i % (MAPD * 16);
		p.y = i / (MAPD * 16);

		cp = nearest_chunk(&p);
		rp = point_sub(&p, &cp);

		switch (map[i]) {
		case ' ':
			valid[*validi] = (p.x << 4) | p.y;
			++(*validi);
			t = tile_plain;
			break;
		default:
			t = tile_sea;
			break;
		}


		ck = get_chunk(cnks, &cp);

		ck->tiles[rp.x][rp.y] = t;
	}
}

#define ITERATIONS 100000

int
main(int argc, const char **argv)
{
	uint8_t valid[256] = { 0 };
	uint32_t valid_len = 0;

	struct chunks cnks = { 0 };
	chunks_init(&cnks);

	parse_map(&cnks, valid, &valid_len);
	ag_init_components(&cnks);

	log_init();
	log_level = ll_debug;

	struct ag_component *agc = hdarr_get_by_i(&cnks.ag.components, 0);
	assert(agc);

	uint8_t path[MAXPATH_LOCAL], pathlen;

	uint32_t i, s, g, possible = 0, impossible = 0;
	for (i = 0; i < ITERATIONS; ++i) {
		s = valid[rand() % valid_len];
		g = valid[rand() % valid_len];
		if (astar_local(agc, s, g, path, &pathlen)) {
			++possible;
		} else {
			++impossible;
		}
	}

	L(log_misc, "finished %d iterations, %d possible, %d impossible", ITERATIONS,
		possible, impossible);
}
