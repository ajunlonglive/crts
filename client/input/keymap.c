#include <stdlib.h>

#include "client/input/keymap.h"

void
keymap_init(struct keymap *km)
{
	km->map = calloc(ASCII_RANGE, sizeof(struct keymap));
}
