#include "shared/constants/blueprints.h"

enum bitmask_lens {
	bl_1  = 0x00000001, bl_2  = 0x00000003, bl_3  = 0x00000007, bl_4  = 0x0000000f,
	bl_5  = 0x0000001f, bl_6  = 0x0000003f, bl_7  = 0x0000007f, bl_8  = 0x000000ff,
	bl_9  = 0x000001ff, bl_10 = 0x000003ff, bl_11 = 0x000007ff, bl_12 = 0x00000fff,
	bl_13 = 0x00001fff, bl_14 = 0x00003fff, bl_15 = 0x00007fff, bl_16 = 0x0000ffff,
	bl_17 = 0x0001ffff, bl_18 = 0x0003ffff, bl_19 = 0x0007ffff, bl_20 = 0x000fffff,
	bl_21 = 0x001fffff, bl_22 = 0x003fffff, bl_23 = 0x007fffff, bl_24 = 0x00ffffff,
	bl_25 = 0x01ffffff, bl_26 = 0x03ffffff, bl_27 = 0x07ffffff, bl_28 = 0x0fffffff,
	bl_29 = 0x1fffffff, bl_30 = 0x3fffffff, bl_31 = 0x7fffffff, bl_32 = 0xffffffff,
};

static struct blueprint_block wood_block[] = {
	{ {  0,  0 }, tile_wood },
};

static struct blueprint_block stone_block[] = {
	{ {  0,  0 }, tile_stone },
};

static struct blueprint_block wood_wall_horiz[] = {
	{ { -2,  0 }, tile_wood },
	{ { -1,  0 }, tile_wood },
	{ {  0,  0 }, tile_wood },
	{ {  1,  0 }, tile_wood },
	{ {  2,  0 }, tile_wood },
};

static struct blueprint_block stone_wall_horiz[] = {
	{ { -2,  0 }, tile_stone },
	{ { -1,  0 }, tile_stone },
	{ {  0,  0 }, tile_stone },
	{ {  1,  0 }, tile_stone },
	{ {  2,  0 }, tile_stone },
};

const struct blueprint blueprints[buildings_count] = {
	[bldg_wood_block]         = { wood_block, bl_1 },
	[bldg_wood_wall_horiz]    = { wood_wall_horiz, bl_5 },
	[bldg_stone_block]        = { stone_block, bl_1 },
	[bldg_stone_wall_horiz]   = { stone_wall_horiz, bl_5 },
};
