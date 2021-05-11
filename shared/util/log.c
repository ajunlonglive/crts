#include "posix.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "shared/util/log.h"

static struct {
	FILE *file;
	enum log_level level;
	uint32_t filter;
	bool initialized, clr;
	uint32_t opts;
} log_cfg = {
	.level = log_info,
	.filter = 0xffffffff,
};

static char *log_level_clr[log_level_count] = {
	[log_warn] = "31",
	[log_info] = "34",
	[log_debug] = "0",
};

static char *log_level_name[log_level_count] = {
	[log_warn] = "warn",
	[log_info] = "info",
	[log_debug] = "dbg",
};

static char *log_filter_name[log_filter_count] = {
	[log_misc] = "",
	[log_mem] = "mem",
	[log_net] = "net",
};

static uint32_t log_filter_bit[log_filter_count] = {
	[log_misc] = 1 << 0,
		[log_mem]  = 1 << 1,
		[log_net]  = 1 << 2,
};

#define BUF_LEN 512

thread_local struct {
	char buf[BUF_LEN + 1];
} fmt_buf = { 0 };

const char *
fmt_str(const char *fmt, ...)
{
	return "";
}

void
log_print(const char *file, uint32_t line, const char *func, enum log_level lvl,
	enum log_filter type, const char *fmt, ...)
{
	static char buf[BUF_LEN + 3];

	if (log_cfg.level >= lvl && log_cfg.filter & log_filter_bit[type]) {
		uint32_t len = 0;

		assert(log_cfg.initialized);

		if (log_cfg.clr) {
			len += snprintf(&buf[len], BUF_LEN - len,
				"\033[%sm%s\033[0m:%s ", log_level_clr[lvl],
				log_level_name[lvl], log_filter_name[type]);
		} else {
			len += snprintf(&buf[len], BUF_LEN - len, "%s:%s ",
				log_level_name[lvl], log_filter_name[type]);
		}

		if (log_cfg.opts & log_show_source) {
			if (log_cfg.clr) {
				len += snprintf(&buf[len], BUF_LEN - len,
					"%s:%d [\033[35m%s\033[0m] ", file, line, func);
			} else {
				len += snprintf(&buf[len], BUF_LEN - len,
					"%s:%d [%s] ", file, line, func);
			}
		}

		va_list ap;
		va_start(ap, fmt);
		len += vsnprintf(&buf[len], BUF_LEN - len, fmt, ap);
		va_end(ap);

		if (len < BUF_LEN) {
			buf[len] = '\n';
			buf[len + 1] = 0;
		}

		fputs(buf, log_cfg.file);
	}
}

void
log_plain(enum log_level lvl, enum log_filter type, const char *fmt, ...)
{
	if (log_cfg.level >= lvl && log_cfg.filter & log_filter_bit[type]) {
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
}

void
log_init(void)
{
	char *sll;
	uint64_t ll;

	assert(!log_cfg.initialized);

	if ((sll = getenv("CRTS_LOG_LVL"))) {
		ll = strtoul(sll, NULL, 10);
		log_set_lvl(ll);
	}

	log_cfg.file = stderr;
	log_cfg.clr = true;
	log_cfg.initialized = true;
}

void
log_bytes(const void *src, size_t size)
{
	const char *bytes = src;
	int32_t i, j;

	for (i = size - 1; i >= 0; --i) {
		for (j = 7; j >= 0; --j) {
			fprintf(log_cfg.file, "%c", bytes[i] & (1 << j) ? '1' : '0');
		}
		fprintf(log_cfg.file, " ");
	}
}

void
log_bytes_r(const void *src, size_t size)
{
	const char *bytes = src;
	uint32_t i, j;

	for (i = 0; i < size; ++i) {
		for (j = 0; j < 8; ++j) {
			fprintf(log_cfg.file, "%c", bytes[i] & (1 << j) ? '1' : '0');
		}
		fprintf(log_cfg.file, " ");
	}
}

bool
log_file_is_a_tty(void)
{
	int fd;
	return (fd = fileno(log_cfg.file)) != -1 && isatty(fd) == 1;
}

void
log_set_file(FILE *log_file)
{
	log_cfg.file = log_file;
	log_cfg.clr = log_file_is_a_tty();
}

void
log_set_lvl(enum log_level ll)
{
	if (ll > log_level_count) {
		L(log_misc, "attempted to set log level to invalid value %d (max: %d)", ll, log_level_count);
		return;
	}

	log_cfg.level = ll;
}

void
log_set_opts(enum log_opts opts)
{
	log_cfg.opts = opts;
}
