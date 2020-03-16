#ifndef SHARED_UTIL_LOG_H
#define SHARED_UTIL_LOG_H

#ifdef NDEBUG
#define L(...)
#define LB(src, size)
#define VBASSERT(exp, ...)
#else
#include <assert.h>
#include <stdio.h>

#define L(...) do { \
		fprintf(stderr, "%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
} while (0)

#define LB(src, size) do { \
		fprintf(stderr, "%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); \
		log_bytes(src, size); \
		fprintf(stderr, "\n"); \
} while (0)


#ifdef VALGRIND_BACKTRACE
#include <valgrind/valgrind.h>
#define VBASSERT(exp, ...) do { \
		if (!(exp)) { \
			if (RUNNING_ON_VALGRIND) { \
				VALGRIND_PRINTF_BACKTRACE(__VA_ARGS__); \
			} else { \
				L(__VA_ARGS__); \
			} \
		} \
		assert(exp); \
} while (0)
#else
#define VBASSERT(exp, ...) do { \
		if (!(exp)) { \
			L(__VA_ARGS__); \
		} \
		assert(exp); \
} while (0)
#endif
#endif

void log_bytes(const void *src, size_t size);
#endif
