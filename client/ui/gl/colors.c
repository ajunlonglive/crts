#include "posix.h"

#include "client/ui/gl/colors.h"

struct colors colors = {
	.ent = {
		[et_sand]    = { 1.0, 0.96, 0.42, 1.0 },
		[et_fire]    = { 1.0, 0.21, 0.13, 0.8 },
		[et_wood]    = { 0.93, 0.77, 0.64, 1.0 },
		[et_acid]    = { 0.03, 0.77, 0.04, 0.7 },
		[et_water]    = { 0.4, 0.56, 0.42, 0.4 },
		[et_spring]    = { 0.6, 0.86, 0.62, 1.0 },
	},
	.tile = {
		[tile_sea]      = { 0.4, 0.56, 0.42, 1.0 },
		[tile_coast]    = { 1.0, 0.96, 0.42, 1.0 },
		[tile_plain]    = { 0.44, 0.69, 0.19, 1.0 },
		[tile_tree]     = { 0.0, 0.5, 0.03, 1.0 },
		[tile_rock]     = { 0.1, 0.1, 0.1, 1.0 },
		[tile_dirt]     = { 0.41, 0.28, 0.15, 1.0 },
		[tile_old_tree] = { 0.28, 0.5, 0.0, 1.0 },
		[tile_fire]     = { 1.0, 0.0, 0.0, 1.0 },
		[tile_ash]      = { 0.2, 0.2, 0.2, 1.0 },
	},
	.sky = { 0.4, 0.45, 0.82, 1.0 },
};
