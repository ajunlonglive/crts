#ifndef CLIENT_CFG_KEYMAP_H
#define CLIENT_CFG_KEYMAP_H

#define KEYMAP_CFG "keymap.ini"

#include <stdbool.h>
#include <stdint.h>

#include "client/input/keymap.h"
#include "client/ui/common.h"

bool parse_keymap(struct keymap *km, struct ui_ctx *ui_ctx);
#endif
