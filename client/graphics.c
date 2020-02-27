#include "client/graphics.h"

struct graphics_t graphics = {
	.tiles = {
		[tile_sand]     = { { '~', color_ylw    }, zi_0 },
		[tile_plain]    = { { '.', color_blu    }, zi_0 },
		[tile_forest]   = { { '^', color_grn    }, zi_0 },
		[tile_mountain] = { { '_', color_bg_wte }, zi_0 },
		[tile_peak]     = { { '^', color_bg_wte }, zi_0 }
	},
	.ents = {
		[et_worker]        = { { '@', color_grn }, zi_2 },
		[et_resource_wood] = { { 'w', color_wte }, zi_1 }
	},
	.cursor = { { '!', color_bg_red }, zi_3 }
};
