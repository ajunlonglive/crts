#ifndef SHARED_UTIL_LOG_H
#define SHARED_UTIL_LOG_H

#include "posix.h"

#include <stddef.h>
#include <stdio.h>

enum log_level {
	ll_quiet,
	ll_warn,
	ll_info,
	ll_debug,
};

extern int logfiled;
extern enum log_level log_level;

#define _LOG_H(str) dprintf(logfiled, "[" str "] %s:%d [\033[35m%s\033[0m] ", __FILE__, __LINE__, __func__);

#define _LOG(...) do { \
		dprintf(logfiled, __VA_ARGS__); \
		dprintf(logfiled, "\n"); \
} while (0)

#define LOG_D(...) if (log_level >= ll_debug) { _LOG_H("debug"); _LOG(__VA_ARGS__); }
#define LOG_W(...) if (log_level >= ll_warn) { _LOG_H("\033[31mwarn\033[0m"); _LOG(__VA_ARGS__); }
#define LOG_I(...) if (log_level >= ll_info) { _LOG_H("\033[34minfo\033[0m"); _LOG(__VA_ARGS__); }

// TODO deprecate this
#define L(...) LOG_D(__VA_ARGS__)

void log_bytes(const void *src, size_t size);
void set_log_file(const char *otparg);
#endif
