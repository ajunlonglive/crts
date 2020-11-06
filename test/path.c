#include "posix.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/local.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

#define X tile_deep_water
#define _ tile_plain

struct chunk ck = {
	.pos = { 16, 16 },
	.tiles = { _, _, _, _, _, _, _, X, _, _, _, _, _, _, _, _,
		   _, _, _, _, _, _, _, X, _, _, _, _, _, _, _, _,
		   _, _, _, _, _, _, _, X, _, _, _, _, _, _, _, _,
		   _, _, X, X, X, X, X, X, _, _, _, _, _, _, _, _,
		   _, _, X, _, _, _, _, _, _, _, _, _, _, _, _, _,
		   _, _, X, _, _, _, _, _, _, _, _, _, _, _, _, _,
		   _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
		   _, _, X, _, _, _, _, _, _, _, _, _, _, _, _, _,
		   _, X, _, _, _, _, _, X, X, X, X, X, _, X, X, X,
		   X, _, _, _, _, _, _, X, _, _, _, _, _, X, _, _,
		   _, _, _, _, _, X, X, X, _, _, _, _, _, X, _, _,
		   _, _, _, _, _, X, _, _, _, _, _, _, _, X, _, _,
		   _, _, _, _, _, X, _, _, _, _, _, _, _, X, _, _,
		   _, _, _, _, _, X, _, _, _, _, _, _, _, X, _, _,
		   _, _, _, _, _, X, _, _, _, _, _, _, _, X, _, _,
		   _, _, _, _, _, X, _, _, _, _, _, _, _, _, _, _, },
};

static void
set_adj_chunks(struct chunks *cnks, struct chunk *ck)
{
	struct chunk empty = { .pos = ck->pos };
	uint32_t i;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		((enum tile *)empty.tiles)[i] = tile_plain;
	}

	empty.pos.x -= CHUNK_SIZE;
	set_chunk(cnks, &empty);

	empty.pos.x += 2 * CHUNK_SIZE;
	set_chunk(cnks, &empty);

	empty.pos.x = ck->pos.x;
	empty.pos.y -= CHUNK_SIZE;
	set_chunk(cnks, &empty);

	empty.pos.y += 2 * CHUNK_SIZE;
	set_chunk(cnks, &empty);
}

void
test_abstract_pathfinding(struct chunks *cnks, struct point *s, struct point *g)
{
	uint32_t i;
	uint16_t pathlen = 0;
	struct ag_path path = { 0 };

	if (!astar_abstract(&cnks->ag, s, g, &path, &pathlen)) {
		L("no path found");
	}

	struct point q;
	for (i = 0; i < pathlen; ++i) {
		q = path.comp[i];
		q.x += path.node[i] % 16;
		q.y += path.node[i] / 16;

		L("comp: (%d, %d) node: %d | pos: (%d, %d)", path.comp[i].x,
			path.comp[i].y, path.node[i], q.x, q.y);
	}
}

int
main(const int argv, const char **argc)
{
	log_init();
	struct chunks cnks = { 0 };
	chunks_init(&cnks);
	set_adj_chunks(&cnks, &ck);
	set_chunk(&cnks, &ck);

	uint32_t i, path;
	for (i = 0; i < hdarr_len(cnks.hd); ++i) {
		ag_preprocess_chunk(&cnks, hdarr_get_by_i(cnks.hd, i));
	}

	struct point s = { 5, 30 }, g = { 26, 25 };
	enum result r;

	test_abstract_pathfinding(&cnks, &s, &g);

	if (hpa_start(&cnks, &s, &g, &path)) {
		while ((r = hpa_continue(&cnks, path, &s)) == rs_cont) {
			L("(%d, %d)", s.x, s.y);
		}
	}
}
