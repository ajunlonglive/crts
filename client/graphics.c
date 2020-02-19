#include "client/graphics.h"

const struct graphics_t graphics = {
	.tiles = {
		[tile_sand]     = { '~', color_ylw },
		[tile_plain]    = { '.', color_blu },
		[tile_forest]   = { '^', color_grn },
		[tile_mountain] = { '_', color_bg_wte },
		[tile_peak]     = { '^', color_bg_wte }
	},
	.ents = {
		[et_worker]        = { '@', color_no },
		[et_resource_wood] = { 'w', color_no }
	},
	.cursor = { '!', color_bg_red },
};
