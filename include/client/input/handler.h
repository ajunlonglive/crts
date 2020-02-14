#ifndef __INPUT_H
#define __INPUT_H
#include "client/cfg/keymap.h"
#include "client/hiface.h"

struct keymap *handle_input(struct keymap *km, unsigned k, struct hiface *hif);
#endif
