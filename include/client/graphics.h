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

enum cursor_type {
	ct_default,
	ct_blueprint_valid,
	ct_blueprint_invalid,
	ct_arrow_up,
	ct_arrow_down,
	ct_arrow_left,
	ct_arrow_right,
	ct_harvest,
	cursor_type_count
};

enum addl_ent_types {
	et_elf_friend = ent_type_count + 1,
	et_elf_foe,
	extended_ent_type_count,
};

_Static_assert(et_elf_foe == et_elf_friend + 1,
	"addl ent types overlaps normal ent types");

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
