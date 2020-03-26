#ifndef SHARED_SIM_ACTION_H
#define SHARED_SIM_ACTION_H
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"

enum action_type {
	at_none,
	at_move,
	at_harvest,
	at_build,
	at_fight,
	at_carry,
	action_type_count
};

struct action {
	struct circle range;
	struct circle source;
	uint16_t tgt;
	enum action_type type;
	uint8_t workers_requested;
	uint8_t id;

#ifdef CRTS_SERVER
	uint8_t motivator;
	uint8_t workers_assigned;
	uint8_t workers_waiting;
	uint8_t completion;
#endif
};

void action_init(struct action *act);
void action_inspect(struct action *act);
#endif
