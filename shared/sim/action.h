#ifndef __ACTION_H
#define __ACTION_H
#include <stdlib.h>
#include "math/geom.h"

enum action_type {
	action_type_0,
	action_type_1,
};

struct action {
	enum action_type type;
	int motivator;
	int id;
	int workers;
	int workers_in_range;
	int completion;

	struct circle range;
};

void action_init(struct action *act);
void action_inspect(struct action *act);
#endif
