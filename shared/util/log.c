#include "posix.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "shared/util/log.h"

int logfiled = STDERR_FILENO;
enum log_level log_level = ll_debug;

void
log_bytes(const void *src, size_t size)
{
	const char *bytes = src;
	int i;

	for (i = size - 1; i >= 0; --i) {
		dprintf(logfiled, "%02hhx", bytes[i]);
	}
}
