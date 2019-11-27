#ifndef __ACTION_H
#define __ACTION_H
#include <stdlib.h>
#include "geom.h"

struct action_info {
	const char *name;
	int max_workers;
	int min_workers;
	int completed_at;
	int satisfaction;
};

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

struct action_update {
	enum action_type type;
	struct circle range;
};

#include "update.h"

void action_init(struct action *act);
void action_inspect(struct action *act);
void action_init_from_update(struct action *act, struct action_update *au);
#endif
