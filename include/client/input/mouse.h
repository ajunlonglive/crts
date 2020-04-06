#ifndef CLIENT_INPUT_MOUSE_H
#define CLIENT_INPUT_MOUSE_H

#include <stdint.h>

#include "client/hiface.h"

void handle_mouse(int x, int y, uint64_t bstate, struct hiface *hf);
#endif
