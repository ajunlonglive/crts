#include "client/display/window.h"
#include "client/graphics.h"

struct graphics_t graphics = { 0 };

void
init_tile_curs(void)
{
	size_t i;

	for (i = 0; i < tile_count; ++i) {
		graphics.tile_curs[i] = graphics.tiles[i];

		graphics.tile_curs[i].pix.attr = graphics.cursor[ct_harvest].pix.attr;
		graphics.tile_curs[i].zi = zi_3;
	}
}
