#ifndef CLIENT_GRAPHICS_H
#define CLIENT_GRAPHICS_H

#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"

#define CHAR_TRANS 1

enum z_index {
	zi_0,
	zi_1,
	zi_2,
	zi_3,
	z_index_count,
};

struct pixel {
	char c;
	uint32_t clr;
	uint32_t attr;
};

struct graphics_info_t {
	struct pixel pix;
	enum z_index zi;
};

struct graphics_t {
	struct graphics_info_t tiles[tile_count];
	struct graphics_info_t tile_curs[tile_count];
	struct graphics_info_t ents[ent_type_count];
	struct graphics_info_t ents_motivated[2];

	struct graphics_info_t cursor;
	struct {
		struct graphics_info_t valid;
		struct graphics_info_t invalid;
	} blueprint;

	struct {
		struct graphics_info_t up;
		struct graphics_info_t down;
		struct graphics_info_t left;
		struct graphics_info_t right;
	} arrow;
};

extern struct graphics_t graphics;

void init_graphics(void);
#endif
