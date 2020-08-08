#include "posix.h"

#include <stdlib.h>

#include "shared/util/log.h"
#include "shared/util/mem.h"

#define DEFAULT_MEM_SIZE 255

void
ensure_mem_size(void **elem, size_t size, size_t len, size_t *cap)
{
	if (len > *cap) {
		if (*cap == 0) {
			*cap = len > DEFAULT_MEM_SIZE ? len : DEFAULT_MEM_SIZE;
		} else {
			*cap = *cap * 2;
		}
		*elem = realloc(*elem, *cap * size);
		if (!*elem) {
			LOG_W("realloc failed");
		}
	}
}

size_t
get_mem(void **elem, size_t size, size_t *len, size_t *cap)
{
	ensure_mem_size(elem, size, ++(*len), cap);

	return *len - 1;
}
