#ifndef __SIM_ENT_H
#define __SIM_ENT_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/math/geom.h"

enum ent_type {
	et_worker,
	et_resource_wood,
	ent_type_count
};

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
