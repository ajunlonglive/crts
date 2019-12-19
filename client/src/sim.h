#ifndef __CLIENT_SIM_H
#define __CLIENT_SIM_H
#include <stddef.h>

struct simulation {
	struct queue *outbound;
	struct queue *inbound;
	struct world *w;

	struct {
		struct action *e;
		size_t len;
		size_t cap;
	} actions;

	int run;
};
#endif
