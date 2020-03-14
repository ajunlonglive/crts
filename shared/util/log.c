#define _XOPEN_SOURCE 500

#include <time.h>

#include "shared/util/log.h"

void
log_bytes(const void *src, size_t size)
{
	const char *bytes = src;
	int i;

	for (i = size - 1; i >= 0; --i) {
		fprintf(stderr, "%02hhx", bytes[i]);
	}
}

void
log_timestamp(void)
{
	struct timespec ts;
	static long start_time = 1584157636;

	clock_gettime(CLOCK_REALTIME, &ts);

	fprintf(stderr, "%ld.%05ld", ts.tv_sec - start_time, ts.tv_nsec / 10000);
}
