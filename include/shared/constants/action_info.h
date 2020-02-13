#ifndef __GLOBALS_H
#define __GLOBALS_H
struct action_info {
	const char *name;
	const int max_workers;
	const int min_workers;
	const int completed_at;
	const int satisfaction;
};

extern const struct action_info ACTIONS[];
#endif
