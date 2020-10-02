#ifndef SHARED_SIM_ACTION_H
#define SHARED_SIM_ACTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"

#define ACTION_RANGE_MAX_H 64
#define ACTION_RANGE_MAX_W 64

enum action_type {
	at_none,
	at_move,
	at_harvest,
	at_build,
	at_fight,
	at_carry,
	action_type_count
};

enum action_flags {
	af_repeat = 1 << 0,
	action_flags_count = 1
};

struct action {
	struct rectangle range;
	struct rectangle source;
	enum action_type type;
	uint16_t tgt;
	uint16_t workers_requested;
	uint8_t id;
	uint8_t flags;

#ifdef CRTS_SERVER
	uint16_t workers_assigned;
	uint16_t workers_waiting;
	uint16_t motivator;
	uint8_t completion;
#endif
};

void action_init(struct action *act);
void action_inspect(struct action *act);
#endif
