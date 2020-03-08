#ifndef SERVER_SIM_ACTION_H
#define SERVER_SIM_ACTION_H

#include <stdbool.h>

#include "server/sim/sim.h"
#include "shared/sim/action.h"

struct sim_action {
	struct action act;
	struct pgraph *global;
	struct pgraph *local;
	uint32_t resources;
	struct hash *ent_blacklist;
	struct hash *hash;
};

struct sim_action *action_get(const struct simulation *sim, uint8_t id);
struct sim_action *action_add(struct simulation *sim, const struct action *act);
void action_del(struct simulation *sim, uint8_t id);
bool action_ent_blacklisted(const struct sim_action *sa, const struct ent *e);
void action_ent_blacklist(struct sim_action *sa, const struct ent *e);
#endif
