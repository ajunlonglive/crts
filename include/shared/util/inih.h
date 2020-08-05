#ifndef SHARED_UTIL_INIH_H
#define SHARED_UTIL_INIH_H

#define CFG_LOOKUP_TBL_LEN 64

#include <stdbool.h>
#include <stdint.h>

#include "shared/util/assets.h"

typedef bool ((*inihcb)(void *ctx, const char *sect, const char *k,
			const char *v, uint32_t line));

bool ini_parse(struct file_data *fd, inihcb cb, void *ctx);

struct cfg_lookup_table {
	struct {
		char *str;
		uint32_t t;
	} e[CFG_LOOKUP_TBL_LEN];
};

int32_t cfg_string_lookup(const char *str, struct cfg_lookup_table *tbl);
bool parse_cfg_file(const char *filename, void *ctx, inihcb handler);
bool str_to_bool(const char *str);
float strdeg_to_rad(const char *str);
#endif
