#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>

#define L(...) do { \
		log_timestamp(); \
		fprintf(stderr, "%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
} while (0)

#define LB(src, size) do { \
		fprintf(stderr, "%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); \
		log_bytes(src, size); \
		fprintf(stderr, "\n"); \
} while (0)

void log_bytes(const void *src, size_t size);
void log_timestamp(void);
#endif
