#include "client/display/attr.h"
#include "client/graphics.h"

struct graphics_t graphics;

void
init_graphics(void)
{
	struct graphics_t g = {
		.tiles = {
			[tile_deep_water]   = { { ' ', color_blue } },
			[tile_water]        = { { ' ', color_blue } },
			[tile_sand]         = { { 's', color_yellow } },
			[tile_plain]        = { { '.', color_green } },
			[tile_dirt]         = { { '.', color_magenta } },
			[tile_forest_young] = { { 't', color_green } },
			[tile_forest]       = { { 'T', color_green } },
			[tile_forest_old]   = { { 'T', color_white } },
			[tile_mountain]     = { { 'm', color_bg_white } },
			[tile_peak]         = { { 'M', color_bg_white } },
			[tile_wood]         = { { 'w', color_bg_green } },
			[tile_wood_floor]   = { { '~', color_white } },
			[tile_rock_floor]   = { { '.', color_white } },
			[tile_stone]        = { { 's', color_bg_white } },
			[tile_shrine]       = { { 's', color_bg_magenta } },
		},
		.ents = {
			[et_resource_wood] = { { 'w', color_white }, zi_1 },
			[et_resource_rock] = { { 'r', color_white }, zi_1 },
		},
		.ents_motivated = {
			{ { '@', color_red }, zi_2 },
			{ { '@', color_cyan }, zi_2 },
		},
		.cursor = { { CHAR_TRANS, color_bg_red }, zi_3 },
		.blueprint = {
			.valid =   { { CHAR_TRANS, color_bg_blue, attr.blink }, zi_3 },
			.invalid = { { CHAR_TRANS, color_bg_red, attr.blink }, zi_3 },
		},
		.arrow = {
			.up    = { { '|', color_bg_red, attr.blink }, zi_3 },
			.down  = { { '|', color_bg_red, attr.blink }, zi_3 },
			.left  = { { '-', color_bg_red, attr.blink }, zi_3 },
			.right = { { '-', color_bg_red, attr.blink }, zi_3 },
		},
	};

	size_t i;
	for (i = 0; i < tile_count; ++i) {
		g.tile_curs[i] = g.tiles[i];
		g.tile_curs[i].pix.clr = g.tiles[i].pix.clr | CLR_BG_MASK;
		g.tile_curs[i].zi = zi_3;
	}

	graphics = g;
}
