#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include "perlin.h"
#include "chunk.h"
#include "../../server/src/terrain.h"

int main(int _, const char **v)
{
	int i, j, k;

	srandom(atoi(v[1]));
	perlin_noise_shuf();

	struct world *w = world_init();
	struct point p = { 0, 0 };
	int chunks = 8;
	struct chunk *cnks[chunks], *cnk;

	//init_terrain_gen();

	for (i = 0; i < chunks; i++) {
		cnks[i] = get_chunk(w, &p);
		p.x += CHUNK_SIZE;
	}

	char c;

	for (j = 0; j < CHUNK_SIZE; j++) {
		for (k = 0; k < chunks; k++) {
			cnk = cnks[k];
			if (j == 0) {
				printf("%3d,%3d", cnk->pos.x, cnk->pos.y);
				i = 7;
			} else {
				i = 0;
			}
			for (; i < CHUNK_SIZE; i++) {
				switch (cnk->tiles[i][j]) {
				case tile_empty:
					c = '.';
					break;
				case tile_full:
					c = '_';
					break;
				case tile_a:
					c = 'a';
					break;
				case tile_b:
					c = 'b';
					break;
				case tile_c:
					c = 'c';
					break;
				default:
					c = 'd';
					break;
				}
				printf("%c", c);
			}
		}
		printf("\n");
	}
}

