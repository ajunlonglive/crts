#ifndef __SIM_ENT_H
#define __SIM_ENT_H
#include "shared/math/geom.h"

struct ent {
	int id;
	struct point pos;
	char c;
	struct alignment *alignment;
	int satisfaction;
	int age;

	int task;
	int idle;
};

void ent_init(struct ent *e);
#endif
