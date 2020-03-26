#ifndef SERVER_SIM_ACTION_H
#define SERVER_SIM_ACTION_H

#include <stdbool.h>

#include "server/sim/sim.h"
#include "shared/sim/action.h"

#define SIM_ACTION_CTX_LEN 64ul

struct sim_action {
	struct action act;
	struct pgraph *global;
	struct pgraph *local;
	struct hash *ent_blacklist;
	struct hash *hash;
	char ctx[SIM_ACTION_CTX_LEN];
	bool deleted;
};

struct sim_action *action_get(const struct simulation *sim, uint8_t id);
struct sim_action *action_add(struct simulation *sim, const struct action *act);
void action_del(struct simulation *sim, uint8_t id);
bool action_ent_blacklisted(const struct sim_action *sa, const struct ent *e);
void action_ent_blacklist(struct sim_action *sa, const struct ent *e);
void sim_actions_init(struct simulation *sim);
void actions_flush(struct simulation *sim);
#endif
