#define _XOPEN_SOURCE 500

#include <time.h>
#include <stdio.h>

#include "shared/util/log.h"

FILE *logfile = NULL;

void
log_bytes(const void *src, size_t size)
{
	const char *bytes = src;
	int i;

	for (i = size - 1; i >= 0; --i) {
		fprintf(stderr, "%02hhx", bytes[i]);
	}
}
