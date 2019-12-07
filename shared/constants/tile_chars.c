#include "sim/chunk.h"
#include "constants/tile_chars.h"

const char tile_chars[] = {
	[tile_empty] = '_',
	[tile_full]  = '.',
	[tile_a]     = '^',
	[tile_b]     = '~',
	[tile_c]     = '^'
};
