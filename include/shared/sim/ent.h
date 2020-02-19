#ifndef __SIM_ENT_H
#define __SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/math/geom.h"

struct ent {
	uint8_t id;
	struct point pos;
	struct alignment *alignment;
	uint8_t satisfaction;

	uint8_t task;
	bool idle;
};

void ent_init(struct ent *e);
#endif
