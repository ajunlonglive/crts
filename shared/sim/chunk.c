#include <stdlib.h>
#include <string.h>
#include "util/log.h"
#include "sim/chunk.h"

void chunk_init(struct chunk **c)
{
	if (c == NULL) {
		L("invalid pointer");
		return;
	}

	if (*c == NULL)
		*c = malloc(sizeof(struct chunk));

	memset(*c, 0, sizeof(struct chunk));
	(*c)->empty = 1;
}

static int roundto(int i, int nearest)
{
	int m = i % nearest;

	return m >= 0 ? i - m : i - (nearest + m);
}

struct point nearest_chunk(const struct point *p)
{
	struct point q = {
		roundto(p->x, CHUNK_SIZE),
		roundto(p->y, CHUNK_SIZE)
	};

	return q;
}
