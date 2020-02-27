#ifndef CLIENT_GRAPHICS_H
#define CLIENT_GRAPHICS_H

#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"

enum color {
	color_no,
	color_blk,
	color_red,
	color_grn,
	color_ylw,
	color_blu,
	color_mag,
	color_cyn,
	color_wte,
	color_bg_blk,
	color_bg_red,
	color_bg_grn,
	color_bg_ylw,
	color_bg_blu,
	color_bg_mag,
	color_bg_cyn,
	color_bg_wte
};

enum z_index {
	zi_0,
	zi_1,
	zi_2,
	z_index_count,
	zi_inf
};

struct pixel {
	char c;
	enum color fg;
};

struct graphics_info_t {
	struct pixel pix;
	enum z_index zi;
};

struct graphics_t {
	struct graphics_info_t tiles[tile_count];
	struct graphics_info_t ents[ent_type_count];
	struct graphics_info_t cursor;
};

extern struct graphics_t graphics;
#endif
