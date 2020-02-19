#ifndef __ACTION_H
#define __ACTION_H
#include <stddef.h>
#include <stdint.h>

#include "shared/math/geom.h"

enum action_type {
	at_none,
	at_move,
	at_harvest,
};

struct action {
	enum action_type type;
	struct circle range;
	uint8_t workers_requested;
	uint8_t id;

#ifdef CRTS_SERVER
	uint8_t motivator;

	uint8_t workers_assigned;
	uint8_t workers_in_range;

	uint8_t completion;
#endif
};

void action_init(struct action *act);
void action_inspect(struct action *act);
#endif
