#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include "math/perlin.h"
#include "util/log.h"
#include "sim/chunk.h"
#include "sim/world.h"
#include "../../server/src/sim/terrain.h"
#include "constants/tile_chars.h"

static int tile_to_clr(enum tile t)
{
	switch (t) {
	case tile_sand: return 46;
	case tile_plain:  return 43;
	case tile_forest:     return 42;
	case tile_mountain:     return 44;
	case tile_peak:     return 45;
	default:         return 47;
	}
}

static char tile_to_c(enum tile t)
{
	if (t < 5)
		return tile_chars[t];
	else
		return '!';
}

static void print_row(struct world *w, struct point *p, int cols)
{
	int i, j, k;
	int chunks = cols;
	const struct chunk *cnks[chunks], *cnk;

	for (i = 0; i < chunks; i++) {
		cnks[i] = get_chunk(w->chunks, p);
		p->x += CHUNK_SIZE;
	}

	for (j = 0; j < CHUNK_SIZE; j++) {
		for (k = 0; k < chunks; k++) {
			cnk = cnks[k];
			for (i = 0; i < CHUNK_SIZE; i++)
				printf(
					"\033[1;30;%dm%c\033[0m",
					tile_to_clr(cnk->tiles[i][j]),
					tile_to_c(cnk->tiles[i][j]));
		}
		printf("\n");
	}
}

#define SX -16
#define SY -16

int main(int ac, const char **v)
{
	int rows = 2, cols = 4;

	if (ac < 2) {
		fprintf(stderr, "error; please provide a seed\n");
		return 1;
	}

	if (ac >= 4) {
		rows = atoi(v[2]);
		cols = atoi(v[3]);
	}

	srandom(atoi(v[1]));
	perlin_noise_shuf();

	struct world *w = world_init();
	struct point p = { 0, SY };
	int i;

	for (i = 0; i < rows; i++) {
		p.x = SX;
		p.y += i * CHUNK_SIZE;
		print_row(w, &p, cols);
	}
}

