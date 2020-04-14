#ifndef CLIENT_GRAPHICS_H
#define CLIENT_GRAPHICS_H

#include "client/ui/graphics.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"

struct pixel {
	uint64_t attr;
	uint32_t clr;
	int16_t fg;
	int16_t bg;
	char c;
};

struct graphics_info_t {
	struct pixel pix;
	enum z_index zi;
};

#define TRANS_COLOR -2
#define TRANS_COLOR_BUF 2
#define TRANS_COLORS (0xff + TRANS_COLOR_BUF)

struct graphics_t {
	struct graphics_info_t tiles[tile_count];
	struct graphics_info_t entities[extended_ent_type_count];
	struct graphics_info_t cursor[cursor_type_count];

	struct graphics_info_t tile_curs[tile_count];
	struct graphics_info_t ent_curs[extended_ent_type_count];

	struct {
		uint16_t pairs[TRANS_COLORS];
		int16_t fg_map[TRANS_COLORS];
		int16_t bg_map[TRANS_COLORS];
		size_t fgi;
		size_t bgi;
	} trans_bg;

	uint32_t color_i;
};

extern struct graphics_t graphics;

void init_tile_curs(void);
uint16_t get_bg_pair(int16_t fg, int16_t bg);
#endif
