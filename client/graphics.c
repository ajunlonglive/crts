#include "client/display/attr.h"
#include "client/graphics.h"

struct graphics_t graphics;

void
init_graphics(void)
{
	struct graphics_t g = {
		.tiles = {
			[tile_deep_water] = { { '_', attr.fg.blue,   attr.normal }, zi_0 },
			[tile_water]      = { { '_', attr.fg.blue,   attr.normal }, zi_0 },
			[tile_sand]       = { { 's', attr.fg.yellow, attr.normal }, zi_0 },
			[tile_plain]      = { { '~', attr.fg.green,  attr.normal }, zi_0 },
			[tile_forest]     = { { 'T', attr.fg.green,  attr.normal }, zi_0 },
			[tile_mountain]   = { { 'm', attr.bg.white,  attr.normal }, zi_0 },
			[tile_peak]       = { { 'M', attr.bg.white,  attr.normal }, zi_0 },
			[tile_bldg]       = { { 'h', attr.bg.blue,   attr.normal }, zi_0 },
		},
		.harvest = {
			[aht_forest]     = { { 'T', attr.fg.green, attr.reverse }, zi_3 },
			[aht_mountain]   = { { 'm', attr.fg.white, attr.reverse }, zi_3 },
			[aht_bldg]       = { { 'h', attr.fg.blue,  attr.reverse }, zi_3 },
		},
		.ents = {
			[et_resource_wood] = { { 'w', attr.fg.white, attr.normal }, zi_1 },
		},
		.ents_motivated = {
			{ { '@', attr.fg.red,   attr.normal }, zi_2 },
			{ { '@', attr.fg.cyan,  attr.normal }, zi_2 },
		},
		.cursor = { { CHAR_TRANS, attr.bg.red, attr.normal }, zi_3 },
		.blueprint = { { CHAR_TRANS, attr.bg.blue, attr.blink }, zi_3 },
		.arrow = {
			.up    = { { '|', attr.bg.red, attr.blink }, zi_3 },
			.down  = { { '|', attr.bg.red, attr.blink }, zi_3 },
			.left  = { { '-', attr.bg.red, attr.blink }, zi_3 },
			.right = { { '-', attr.bg.red, attr.blink }, zi_3 },
		},
	};

	graphics = g;
}
