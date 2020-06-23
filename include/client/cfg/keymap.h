#ifndef CLIENT_CFG_KEYMAP_H
#define CLIENT_CFG_KEYMAP_H

#include <stdbool.h>
#include <stdint.h>

#include "client/input/keymap.h"

bool parse_keymap_handler(void *vp, const char *sec, const char *k, const char *v,
	uint32_t line);
bool parse_keymap(struct keymap *km);
#endif
