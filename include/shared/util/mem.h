#ifndef __UTIL_MEM_H
#define __UTIL_MEM_H
#include <stddef.h>

size_t get_mem(void **elem, size_t size, size_t *len, size_t *cap);
void ensure_mem_size(void **elem, size_t size, size_t len, size_t *cap);
#endif
