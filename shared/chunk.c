#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "chunk.h"

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
