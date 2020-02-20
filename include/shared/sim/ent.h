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
	enum ent_type type;
	uint8_t id;
	struct point pos;
#ifdef CRTS_SERVER
	struct alignment *alignment;
#else
	uint8_t alignment;
#endif

#ifdef CRTS_SERVER
	uint8_t satisfaction;
	bool idle;
	uint8_t task;
#endif
};

void ent_init(struct ent *e);
#endif
