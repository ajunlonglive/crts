#include <stdlib.h>
#include <string.h>

#include "shared/sim/chunk.h"
#include "shared/util/log.h"

void
chunks_init(struct chunks **cnks)
{
	if (*cnks == NULL) {
		*cnks = calloc(1, sizeof(struct chunks));
	} else {
		memset(*cnks, 0, sizeof(struct chunks));
	}

	(*cnks)->hd = hdarr_init(2048, sizeof(struct point), sizeof(struct chunk), NULL);
#ifdef CRTS_SERVER
	(*cnks)->functional_tiles = hash_init(256, 1, sizeof(struct point));
	(*cnks)->functional_tiles_buf = hash_init(256, 1, sizeof(struct point));
#endif
}

void
chunks_destroy(struct chunks *cnks)
{
	hdarr_destroy(cnks->hd);

#ifdef CRTS_SERVER
	hash_destroy(cnks->functional_tiles);
	hash_destroy(cnks->functional_tiles_buf);
#endif

	free(cnks);
}

void
chunk_init(struct chunk **c)
{
	if (*c == NULL) {
		*c = calloc(1, sizeof(struct chunks));
	} else {
		memset(*c, 0, sizeof(struct chunk));
	}

	(*c)->empty = 1;
}

struct point
nearest_chunk(const struct point *p)
{
	return point_mod(p, CHUNK_SIZE);
}
