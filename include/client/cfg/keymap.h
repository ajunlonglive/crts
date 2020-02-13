#ifndef __KEYMAP_H
#define __KEYMAP_H
#include "../input/keycmd.h"

#define ASCII_RANGE 128

struct keymap {
	enum key_command cmd;
	struct keymap *map;
};

struct keymap *parse_keymap(const char *filename);
#endif
