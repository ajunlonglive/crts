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
	uint32_t clr;
	uint64_t attr;
	char c;
};

struct graphics_info_t {
	struct pixel pix;
	uint32_t fg;
	uint32_t bg;
	enum z_index zi;
};

struct graphics_t {
	struct graphics_info_t tiles[tile_count];
	struct graphics_info_t entities[extended_ent_type_count];
	struct graphics_info_t cursor[cursor_type_count];

	struct graphics_info_t tile_curs[tile_count];

	uint32_t color_i;
};

extern struct graphics_t graphics;

void init_tile_curs(void);
#endif
