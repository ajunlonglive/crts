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

/* format: index, adjacent chunk index, index within adjacent chunk, index
 * within adjacent chunk perimeter*/
/* adjacent chunks: left, down, right, up */
/* index format: 0: x-col, 1: adjacent chunk, 2: x-col, 3: perim */
const uint32_t ag_component_node_indices[CHUNK_PERIM + 1][4] = {
	/* left */
	{ 0,  adjck_left, 240, 32, },
	{ 1,  adjck_left, 241, 33, },
	{ 2,  adjck_left, 242, 34, },
	{ 3,  adjck_left, 243, 35, },
	{ 4,  adjck_left, 244, 36, },
	{ 5,  adjck_left, 245, 37, },
	{ 6,  adjck_left, 246, 38, },
	{ 7,  adjck_left, 247, 39, },
	{ 8,  adjck_left, 248, 40, },
	{ 9,  adjck_left, 249, 41, },
	{ 10, adjck_left, 250, 42, },
	{ 11, adjck_left, 251, 43, },
	{ 12, adjck_left, 252, 44, },
	{ 13, adjck_left, 253, 45, },
	{ 14, adjck_left, 254, 46, },
	{ 15, adjck_left, 255, 47, },

	/* bottom */
	{ 15,  adjck_down, 0,   48, },
	{ 31,  adjck_down, 16,  49, },
	{ 47,  adjck_down, 32,  50, },
	{ 63,  adjck_down, 48,  51, },
	{ 79,  adjck_down, 64,  52, },
	{ 95,  adjck_down, 80,  53, },
	{ 111, adjck_down, 96,  54, },
	{ 127, adjck_down, 112, 55, },
	{ 143, adjck_down, 128, 56, },
	{ 159, adjck_down, 144, 57, },
	{ 175, adjck_down, 160, 58, },
	{ 191, adjck_down, 176, 59, },
	{ 207, adjck_down, 192, 60, },
	{ 223, adjck_down, 208, 61, },
	{ 239, adjck_down, 224, 62, },
	{ 255, adjck_down, 240, 63, },

	/* right */
	{ 240, adjck_right,  0,  0, },
	{ 241, adjck_right,  1,  1, },
	{ 242, adjck_right,  2,  2, },
	{ 243, adjck_right,  3,  3, },
	{ 244, adjck_right,  4,  4, },
	{ 245, adjck_right,  5,  5, },
	{ 246, adjck_right,  6,  6, },
	{ 247, adjck_right,  7,  7, },
	{ 248, adjck_right,  8,  8, },
	{ 249, adjck_right,  9,  9, },
	{ 250, adjck_right, 10, 10, },
	{ 251, adjck_right, 11, 11, },
	{ 252, adjck_right, 12, 12, },
	{ 253, adjck_right, 13, 13, },
	{ 254, adjck_right, 14, 14, },
	{ 255, adjck_right, 15, 15, },

	/* top */
	{ 0,   adjck_up, 15,  16, },
	{ 16,  adjck_up, 31,  17, },
	{ 32,  adjck_up, 47,  18, },
	{ 48,  adjck_up, 63,  19, },
	{ 64,  adjck_up, 79,  20, },
	{ 80,  adjck_up, 95,  21, },
	{ 96,  adjck_up, 111, 22, },
	{ 112, adjck_up, 127, 23, },
	{ 128, adjck_up, 143, 24, },
	{ 144, adjck_up, 159, 25, },
	{ 160, adjck_up, 175, 26, },
	{ 176, adjck_up, 191, 27, },
	{ 192, adjck_up, 207, 28, },
	{ 208, adjck_up, 223, 29, },
	{ 224, adjck_up, 239, 30, },
	{ 240, adjck_up, 255, 31, },

	/* tmp_node */
	{ -1, adjck_no, -1, -1, },
};

static void
calc_chunk_adjacency_matrix(struct chunk *ck, uint8_t edges[(CHUNK_SIZE * CHUNK_SIZE) / 2],
	enum trav_type tt)
{
	uint32_t i;
	uint8_t trav[(CHUNK_SIZE * CHUNK_SIZE) / 8] = { 0 }, v;

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		SB1_SET(trav, i, tile_is_traversable(((enum tile *)ck->tiles)[i], tt));

		assert(SB1_GET(trav, i) == tile_is_traversable(((enum tile *)ck->tiles)[i], tt));
	}

	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		v = TRAV_LEFT_OF(trav, i)
		    | (TRAV_BELOW(trav, i) << 1)
		    | (TRAV_RIGHT_OF(trav, i) << 2)
		    | (TRAV_ABOVE(trav, i) << 3);

		SB4_SET(edges, i, v);

		assert(v == SB4_GET(edges, i));
	}

	/* for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) { */
	/* 	uint8_t v = SB4_GET(edges, i); */
	/* 	fprintf(stderr, "%x",  v); */

	/* 	if (!((i + 1) & 15)) { */
	/* 		fprintf(stderr, "\n"); */
	/* 	} */
	/* } */
}

static void
add_node(struct ag_component *agc, uint8_t i)
{
	agc->nodes[i].flags = agn_filled;
	/* adj_ck[adj_indices[i][1]]->ag.nodes[adj_indices[i][1]] = */
	/* 	(struct pg_abstract_node){ .flags = pgan_filled }; */
}

static void
add_nodes(struct ag_component *agc, uint8_t elen, uint32_t es,
	uint32_t i)
{
	if (elen >= 4) {
		add_node(agc, i - 1);
		add_node(agc, es);
	} else {
		add_node(agc, es + (elen / 2));
	}
}

static void
get_adj_chunks(struct chunks *cnks, struct chunk *ck, struct chunk *adj_cks[4])
{
	struct point cp = ck->pos;

	cp.x -= CHUNK_SIZE;
	adj_cks[adjck_left] = hdarr_get(cnks->hd, &cp);

	cp.x += 2 * CHUNK_SIZE;
	adj_cks[adjck_right] = hdarr_get(cnks->hd, &cp);

	cp.x = ck->pos.x;
	cp.y -= CHUNK_SIZE;
	adj_cks[adjck_up] = hdarr_get(cnks->hd, &cp);

	cp.y += 2 * CHUNK_SIZE;
	adj_cks[adjck_down] = hdarr_get(cnks->hd, &cp);
}

static void
find_chunk_entrances(struct chunks *cnks, struct chunk *ck, struct ag_component *agc, enum trav_type trav)
{
	uint32_t s, k, i, es;
	uint8_t elen;
	enum tile curt, adjt;
	struct chunk *adj_cks[4] = { 0 }, *adj_ck;

	get_adj_chunks(cnks, ck, adj_cks);

	for (s = 0; s < 4; ++s) {
		if (!(adj_ck = adj_cks[s])) {
			continue;
		}

		elen = 0;

		for (k = 0; k < CHUNK_SIZE; ++k) {
			i = s * CHUNK_SIZE + k;

			curt = ((enum tile *)ck->tiles)[ag_component_node_indices[i][0]];
			adjt = ((enum tile *)adj_ck->tiles)[ag_component_node_indices[i][2]];

			if (tile_is_traversable(curt, trav) && tile_is_traversable(adjt, trav)) {
				if (!elen) {
					es = i;
				}

				++elen;
			} else if (elen) {
				add_nodes(agc, elen, es, i);
				elen = 0;
			}
		}

		if (elen) {
			add_nodes(agc, elen, es, i + 1);
		}
	}

}

static void
add_edge(struct ag_node *n, uint8_t wt, uint8_t i)
{
	assert(n->edges < MAX_INTER_EDGES);

	/* uint8_t j; */
	/* for (j = 0; j < n->edges; ++j) { */
	/* 	if (n->adjacent[n->edges] == i) { */
	/* 		L("dupe!"); */
	/* 	} */
	/* } */

	n->weights[n->edges] = wt;
	n->adjacent[n->edges] = i;
	n->edges++;
}

bool
insert_tmp_node(struct ag_component *agc, uint8_t tmp_node_i)
{
	TracyCZoneAutoS;
	bool connected = false;
	uint16_t i;
	uint8_t plen, path[MAXPATH_LOCAL] = { 0 };

	for (i = 0; i < CHUNK_PERIM; ++i) {
		if (!(agc->nodes[i].flags & agn_filled)) {
			continue;
		}
		/* TODO: is this a special case? */
		/* } else if (ag_component_node_indices[i][0] == tmp_node_i) { */
		/* 	L("tmp node is already on the perimiter"); */

		if ((astar_local(agc, ag_component_node_indices[i][0], tmp_node_i, path, &plen))) {
			connected = true;
			add_edge(&agc->nodes[i], plen, tmp_node);
			add_edge(&agc->nodes[tmp_node], plen, i);
		}
	}

	TracyCZoneAutoE;
	return connected;
}

static void
find_inter_edge(struct ag_component *agc, uint8_t i, uint8_t j)
{
	uint8_t path[MAXPATH_LOCAL] = { 0 }, plen;

	/* L("checking %d and %d", i, j); */
	if ((astar_local(agc, ag_component_node_indices[i][0],
		ag_component_node_indices[j][0], path, &plen))) {
		/* L("connected %d and %d", i, j); */

		add_edge(&agc->nodes[i], plen, j);
		add_edge(&agc->nodes[j], plen, i);
	}
}

static void
find_inter_edges(struct ag_component *agc)
{
	uint8_t i, j;

	for (i = 0; i < CHUNK_PERIM; ++i) {
		if (!(agc->nodes[i].flags & agn_filled)) {
			continue;
		}

		for (j = i + 1; j < CHUNK_PERIM; ++j) {
			/* for (j = 0; j < CHUNK_PERIM; ++j) { */
			if (!(agc->nodes[j].flags & agn_filled)) {
				continue;
			}

			find_inter_edge(agc, i, j);
		}
	}
}


void
ag_preprocess_chunk(struct chunks *cnks, struct chunk *ck)
{
	struct ag_component *agc;
	if (!(agc = hdarr_get(cnks->ag.components, &ck->pos))) {
		struct ag_component empty = { .pos = ck->pos };
		/* L("setting agc @ (%d, %d)", ck->pos.x, ck->pos.y); */
		hdarr_set(cnks->ag.components, &ck->pos, &empty);
		agc = hdarr_get(cnks->ag.components, &ck->pos);
	}

	assert(agc);

	calc_chunk_adjacency_matrix(ck, agc->edges, cnks->ag.trav);
	/* L("finding chunk entrances"); */
	find_chunk_entrances(cnks, ck, agc, cnks->ag.trav);
	/* L("finding inter edges"); */
	find_inter_edges(agc);

/* 	uint8_t i, j, k = 0; */
/* 	for (i = 0; i < CHUNK_PERIM; ++i) { */
/* 		if (agc->nodes[i].flags & agn_filled) { */
/* 			L("  node @ %d", i); */
/* 			for (j = 0; j < agc->nodes[i].edges; ++j) { */
/* 				L("  %d: %d", agc->nodes[i].adjacent[j], */
/* 					agc->nodes[i].weights[j]); */
/* 			} */
/* 			k = 1; */
/* 		} */
/* 	} */

	/* if (k) { */
	/* 	assert(false); */
	/* } */
}
