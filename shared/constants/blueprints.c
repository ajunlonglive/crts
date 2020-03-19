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

static struct blueprint_block wood_block[] = { { {  0,  0 }, tile_wood } };

static struct blueprint_block stone_block[] = { { {  0,  0 }, tile_stone } };

#define WALL(name, t) \
	static struct blueprint_block name ## _horiz[] = { \
		{ { -2,  0 }, t }, \
		{ { -1,  0 }, t }, \
		{ {  0,  0 }, t }, \
		{ {  1,  0 }, t }, \
		{ {  2,  0 }, t }, \
	}; \
	static struct blueprint_block name ## _vert[] = { \
		{ { 0, -2 }, t }, \
		{ { 0, -1 }, t }, \
		{ { 0,  0 }, t }, \
		{ { 0,  1 }, t }, \
		{ { 0,  2 }, t }, \
	};

WALL(wood_wall, tile_wood);
WALL(stone_wall, tile_stone);
#undef WALL

const struct blueprint blueprints[buildings_count] = {
	[bldg_wood_block]                = { "wood block",  wood_block, bl_1 },
	[bldg_wood_block | bldg_rotate ] = { "wood block",  wood_block, bl_1 },
	[bldg_stone_block]               = { "stone block", stone_block, bl_1 },
	[bldg_stone_block | bldg_rotate] = { "stone block", stone_block, bl_1 },
	[bldg_wood_wall]                 = { "wood wall",   wood_wall_horiz, bl_5 },
	[bldg_wood_wall | bldg_rotate]   = { "wood wall",   wood_wall_vert, bl_5 },
	[bldg_stone_wall]                = { "stone wall",  stone_wall_horiz, bl_5 },
	[bldg_stone_wall | bldg_rotate]  = { "stone wall",  stone_wall_vert, bl_5 },
};
