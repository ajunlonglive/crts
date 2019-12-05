#ifndef __INPUT_H
#define __INPUT_H
#include "../cfg/keymap.h"
#include "../display.h"

struct keymap *handle_input(struct keymap *km, unsigned k, struct display *disp);
#endif
