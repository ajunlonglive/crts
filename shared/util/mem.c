#include <stdlib.h>

#include "shared/util/log.h"
#include "shared/util/mem.h"

#define DEFAULT_MEM_SIZE 255

size_t
get_mem(void **elem, size_t size, size_t *len, size_t *cap)
{
	(*len)++;

	if (*len > *cap) {
		*cap = *cap <= 0 ? DEFAULT_MEM_SIZE : *cap * 2;
		*elem = realloc(*elem, *cap * size);
	}

	return *len - 1;
}
