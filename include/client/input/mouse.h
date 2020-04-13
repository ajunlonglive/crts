#ifndef CLIENT_INPUT_MOUSE_H
#define CLIENT_INPUT_MOUSE_H

#include <stdint.h>

#include "client/hiface.h"

enum mouse_state {
	ms_b1_press   = 1 << 0,
	ms_b1_release = 1 << 1,
	ms_b3_press   = 1 << 2,
	ms_b3_release = 1 << 3,
};

void handle_mouse(int x, int y, uint64_t bstate, struct hiface *hf);
#endif
