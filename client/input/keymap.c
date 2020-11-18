#include "posix.h"

#include <stdlib.h>

#include "client/input/keymap.h"
#include "shared/util/mem.h"

const char *input_mode_names[input_mode_count] = {
	[im_normal] = "normal",
	[im_select] = "select",
	[im_resize] = "resize",
	[im_cmd]    = "command",
};

/* TODO: revisit keymap structure.  It can probably do with an overhaul */
void
keymap_init(struct keymap *km)
{
	km->map = z_calloc(ASCII_RANGE, sizeof(struct keymap));
}
