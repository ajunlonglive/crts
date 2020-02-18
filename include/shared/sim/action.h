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
	uint8_t motivator;
	uint8_t id;

	struct {
		uint8_t requested;
		uint8_t assigned;
		uint8_t in_range;
	} workers;

	uint8_t completion;

	struct circle range;
};

void action_init(struct action *act);
void action_inspect(struct action *act);
#endif
