#ifndef __CLIENT_SIM_H
#define __CLIENT_SIM_H

#include <stdbool.h>
#include <stddef.h>

struct simulation {
	struct queue *outbound;
	struct queue *inbound;
	struct world *w;

	struct {
		bool chunks;
		bool ents;
	} changed;

	struct {
		struct action *e;
		size_t len;
		size_t cap;
	} actions;

	int run;
};
#endif
