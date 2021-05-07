#include "posix.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "shared/util/log.h"

FILE *logfile = NULL;
enum log_level log_level = ll_info;
bool logging_initialized = false;

void
log_init(void)
{
	char *sll;
	uint64_t ll;

	assert(!logging_initialized);

	if ((sll = getenv("CRTS_LOG_LVL"))
	    && (ll = strtoul(sll, NULL, 10)) < log_level_count) {
		log_level = ll;
	}

	logfile = stderr;
	logging_initialized = true;
}

void
log_bytes(const void *src, size_t size)
{
	const char *bytes = src;
	int32_t i, j;

	for (i = size - 1; i >= 0; --i) {
		for (j = 7; j >= 0; --j) {
			fprintf(logfile, "%c", bytes[i] & (1 << j) ? '1' : '0');
		}
		fprintf(logfile, " ");
	}
}

void
log_bytes_r(const void *src, size_t size)
{
	const char *bytes = src;
	uint32_t i, j;

	for (i = 0; i < size; ++i) {
		for (j = 0; j < 8; ++j) {
			fprintf(logfile, "%c", bytes[i] & (1 << j) ? '1' : '0');
		}
		fprintf(logfile, " ");
	}
}

void
set_log_file(const char *path)
{
	FILE *f;
	if (!(f = fopen(path, "wb"))) {
		LOG_W("failed to open logfile '%s': %s",
			strerror(errno), path);
	} else {
		logfile = f;
	}
}

void
set_log_lvl(const char *otparg)
{
	log_level = strtol(optarg, NULL, 10);
}
