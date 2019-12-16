#ifndef __UTIL_MEM_H
#define __UTIL_MEM_H
#include <stddef.h>

void *get_mem(void **elem, size_t size, size_t *len, size_t *cap);
#endif
