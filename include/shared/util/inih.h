#ifndef SHARED_UTIL_INIH_H
#define SHARED_UTIL_INIH_H

#define CFG_LOOKUP_TBL_LEN 256
#define INIH_ERR_LEN 256

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define INIH_ERR(...) snprintf(err, INIH_ERR_LEN, __VA_ARGS__)


#include "shared/util/assets.h"

typedef bool ((*inihcb)(void *ctx, char err[INIH_ERR_LEN], const char *sect, const char *k,
			const char *v, uint32_t line));

bool ini_parse(struct file_data *fd, inihcb cb, void *ctx);

struct cfg_lookup_table {
	struct {
		char *str;
		uint32_t t;
	} e[CFG_LOOKUP_TBL_LEN];
};

int32_t cfg_string_lookup(const char *str, const struct cfg_lookup_table *tbl);
int32_t cfg_string_lookup_n(const char *str, const struct cfg_lookup_table *tbl, uint32_t n);
bool parse_cfg_file(const char *filename, void *ctx, inihcb handler);
bool str_to_bool(const char *str);
float strdeg_to_rad(const char *str);
#endif
