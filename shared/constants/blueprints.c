#include "posix.h"

#include "shared/constants/blueprints.h"

enum bitmask_lens {
	bl_1  = 0x00000001, bl_2  = 0x00000003, bl_3  = 0x00000007, bl_4  = 0x0000000f,
	bl_5  = 0x0000001f, bl_6  = 0x0000003f, bl_7  = 0x0000007f, bl_8  = 0x000000ff,
	bl_9  = 0x000001ff, bl_10 = 0x000003ff, bl_11 = 0x000007ff, bl_12 = 0x00000fff,
	bl_13 = 0x00001fff, bl_14 = 0x00003fff, bl_15 = 0x00007fff, bl_16 = 0x0000ffff,
	bl_17 = 0x0001ffff, bl_18 = 0x0003ffff, bl_19 = 0x0007ffff, bl_20 = 0x000fffff,
	bl_21 = 0x001fffff, bl_22 = 0x003fffff, bl_23 = 0x007fffff, bl_24 = 0x00ffffff,
	bl_25 = 0x01ffffff, bl_26 = 0x03ffffff, bl_27 = 0x07ffffff, bl_28 = 0x0fffffff,
	bl_29 = 0x1fffffff, bl_30 = 0x3fffffff, bl_31 = 0x7fffffff
};

static struct blueprint_block fire[] = { { {  0,  0 }, tile_burning } };

static struct blueprint_block wood_block[] = { { {  0,  0 }, tile_wood } };

static struct blueprint_block stone_block[] = { { {  0,  0 }, tile_stone } };

static struct blueprint_block wood_floor[] = { { {  0,  0 }, tile_wood_floor } };

static struct blueprint_block shrine[] = { { {  0,  0 }, tile_shrine } };

static struct blueprint_block wood_floor_2x2[] = {
	{ {  0,  0 }, tile_wood_floor },
	{ {  1,  0 }, tile_wood_floor },
	{ {  0,  1 }, tile_wood_floor },
	{ {  1,  1 }, tile_wood_floor },
};

static struct blueprint_block farm[] = {
	{ {  0,  0 }, tile_farmland_empty },
	{ {  1,  0 }, tile_farmland_empty },
	{ {  0,  1 }, tile_farmland_empty },
	{ {  1,  1 }, tile_farmland_empty },
};


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

WALL(wood_wall, tile_wood)
WALL(stone_wall, tile_stone)
#undef WALL

#define SYMMETRIC(name, str, len) \
	[bldg_ ## name]                = { str, name, len }, \
	[bldg_ ## name | bldg_rotate ] = { str, name, len }
#define ASYMMETRIC(name, str, len) \
	[bldg_ ## name]                = { str,     name ## _horiz, len }, \
	[bldg_ ## name | bldg_rotate]  = { str " r", name ## _vert,  len }

const struct blueprint blueprints[buildings_count] = {
	ASYMMETRIC(stone_wall, "stone wall", bl_5),
	ASYMMETRIC(wood_wall, "wood wall", bl_5),
	SYMMETRIC(shrine, "shrine", bl_1),
	SYMMETRIC(stone_block, "stone block", bl_1),
	SYMMETRIC(wood_block, "wood block", bl_1),
	SYMMETRIC(wood_floor, "wood floor", bl_1),
	SYMMETRIC(wood_floor_2x2, "wood floor 2x2", bl_4),
	SYMMETRIC(farm, "farm", bl_4),
	SYMMETRIC(fire, "fire", bl_1),
};
