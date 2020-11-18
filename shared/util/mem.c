#include "posix.h"

#include <assert.h>
#include <stdlib.h>

#include "shared/util/mem.h"
#include "tracy.h"

void *
z_malloc(size_t size)
{
	void *r = malloc(size);
	TracyCAlloc(r, size);
	assert(r);
	return r;
}

void *
z_calloc(size_t nmemb, size_t size)
{
	void *r = calloc(nmemb, size);
	TracyCAlloc(r, size);
	assert(r);
	return r;
}

void *
z_realloc(void *ptr, size_t size)
{
	void *r = realloc(ptr, size);
	TracyCFree(ptr);
	TracyCAlloc(r, size);
	assert(r);
	return r;
}

void
z_free(void *ptr)
{
	TracyCFree(ptr);
	free(ptr);
}
