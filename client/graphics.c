#include "client/graphics.h"

struct graphics_t graphics = {
	.tiles = {
		[tile_deep_water] = { { 'W', color_blu    }, zi_0 },
		[tile_water]      = { { 'w', color_blu    }, zi_0 },
		[tile_sand]       = { { '.', color_ylw    }, zi_0 },
		[tile_plain]      = { { '~', color_mag    }, zi_0 },
		[tile_forest]     = { { '^', color_grn }, zi_0 },
		[tile_mountain]   = { { 'm', color_bg_wte }, zi_0 },
		[tile_peak]       = { { 'M', color_bg_wte }, zi_0 },
		[tile_bldg]       = { { 'h', color_bg_blu }, zi_0 },
	},
	.ents = {
		[et_worker]        = { { '@', color_grn }, zi_2 },
		[et_resource_wood] = { { 'w', color_wte }, zi_1 },
	},
	.cursor = { { '!', color_bg_red }, zi_3 }
};
