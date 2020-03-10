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

#ifdef CRTS_SERVER
	(*cnks)->repathfind = hash_init(32, 1, sizeof(struct point));
#endif

	(*cnks)->hd = hdarr_init(2048, sizeof(struct point), sizeof(struct chunk), NULL);
}

void
chunks_destroy(struct chunks *cnks)
{
#ifdef CRTS_SERVER
	hash_destroy(cnks->repathfind);
#endif

	hdarr_destroy(cnks->hd);
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
