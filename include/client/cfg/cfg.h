#ifndef CLIENT_CFG_CFG_H
#define CLIENT_CFG_CFG_H
#define LOOKUP_TBL_LEN 64

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "client/cfg/ini.h"
#include "client/input/keymap.h"
#include "client/opts.h"
#include "client/ui/ncurses/graphics.h"

struct lookup_table {
	struct {
		char *str;
		uint32_t t;
	} e[LOOKUP_TBL_LEN];
};

int32_t cfg_string_lookup(const char *str, struct lookup_table *tbl);
bool cfg_parse_graphics(char *path, struct graphics_t *g);
bool cfg_parse_keymap(char *path, struct keymap *km);
#endif
