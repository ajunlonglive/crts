#ifndef __INPUT_H
#define __INPUT_H
#include "../cfg/keymap.h"
#include "../hiface.h"

struct keymap *handle_input(struct keymap *km, unsigned k, struct hiface *hif);
#endif
