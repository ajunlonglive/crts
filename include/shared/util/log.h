#ifndef SHARED_UTIL_LOG_H
#define SHARED_UTIL_LOG_H

#include <stddef.h>
#include <stdio.h>

extern FILE *logfile;

#ifdef NDEBUG
#define L(...)
#else
#define L(...) do { \
		fprintf(logfile, "%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); \
		fprintf(logfile, __VA_ARGS__); \
		fprintf(logfile, "\n"); \
} while (0)
#endif

void log_bytes(const void *src, size_t size);
#endif
