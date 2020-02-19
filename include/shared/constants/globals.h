#ifndef CRTS_GLOBALS_H
#define CRTS_GLOBALS_H

#include "shared/sim/action.h"

struct global_cfg_t {
	const struct {
		const char *name;
		const int max_workers;
		const int min_workers;
		const int completed_at;
		const int satisfaction;
	} actions[action_type_count];
};

extern const struct global_cfg_t gcfg;
#endif
