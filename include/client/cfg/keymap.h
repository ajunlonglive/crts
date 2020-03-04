#ifndef __KEYMAP_H
#define __KEYMAP_H

#include <stdint.h>

#include "client/input/keycmd.h"

#define ASCII_RANGE 128
#define KEYMAP_MACRO_LEN 32

struct keymap {
	enum key_command cmd;
	char strcmd[KEYMAP_MACRO_LEN];
	struct keymap *map;
};

struct keymap *parse_keymap(const char *filename);
#endif
