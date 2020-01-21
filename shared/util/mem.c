#include <stdlib.h>
#include "util/mem.h"
#include "util/log.h"

#define DEFAULT_MEM_SIZE 255

size_t
get_mem(void **elem, size_t size, size_t *len, size_t *cap)
{
	(*len)++;

	if (*len > *cap) {
		*cap = *cap <= 0 ? DEFAULT_MEM_SIZE : *cap * 2;
		*elem = realloc(*elem, *cap * size);
		/*
		   for (size_t i = 0; i < *cap; i++)
		        L("  %d => %p", i, *elem + (size * i));
		 */
	}

	return *len - 1;
}
