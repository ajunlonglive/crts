#include "posix.h"

#include <stdlib.h>

#include "client/input/keymap.h"

const char *input_mode_names[input_mode_count] = {
	[im_normal] = "normal",
	[im_select] = "select",
	[im_resize] = "resize",
};

void
keymap_init(struct keymap *km)
{
	km->map = calloc(ASCII_RANGE, sizeof(struct keymap));
}
