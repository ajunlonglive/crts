#ifndef CLIENT_SIM_H
#define CLIENT_SIM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "shared/sim/action.h"

#define ACTION_HISTORY_SIZE 256

struct simulation {
	struct queue *outbound;
	struct queue *inbound;
	struct world *w;

	uint8_t assigned_motivator;

	struct {
		size_t ents;
	} server_world;

	struct {
		bool chunks;
		bool ents;
		bool actions;
	} changed;

	struct action action_history[ACTION_HISTORY_SIZE];
	uint8_t action_history_order[ACTION_HISTORY_SIZE];
	size_t action_history_len;

	int run;
};
#endif
