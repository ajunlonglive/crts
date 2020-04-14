#ifndef CLIENT_UI_GRAPHICS_H
#define CLIENT_UI_GRAPHICS_H

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
	ct_crosshair,
	ct_crosshair_dim,
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
#endif
