#include "posix.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "shared/util/log.h"

FILE *logfile = NULL;
enum log_level log_level = ll_info;

void
log_bytes(const void *src, size_t size)
{
	const char *bytes = src;
	int i;

	for (i = size - 1; i >= 0; --i) {
		fprintf(logfile, "%02hhx", bytes[i]);
	}
}

void
set_log_file(const char *path)
{
	FILE *f;
	if (!(f = fopen(path, "w"))) {
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
