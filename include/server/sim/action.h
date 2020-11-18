#ifndef SERVER_SIM_ACTION_H
#define SERVER_SIM_ACTION_H

#include <stdbool.h>

#include "server/sim/ent_lookup.h"
#include "server/sim/sim.h"
#include "shared/net/connection.h"
#include "shared/pathfind/pgraph.h"
#include "shared/sim/action.h"

#define SIM_ACTION_CTX_LEN 512

enum sim_action_state {
	sas_new     = 1 << 0,
	sas_deleted = 1 << 1,
};

struct sim_action;

typedef enum result ((*do_action_fn)(struct simulation *sim, struct ent *e, struct sim_action *act));
typedef enum result ((*do_action_teardown_fn)(struct sim_action *act));

struct sim_action {
	uint8_t ctx[SIM_ACTION_CTX_LEN];
	struct ent_lookup_ctx elctx;
	struct action act;
	struct hash exclude;

	do_action_fn do_action;
	do_action_teardown_fn do_action_teardown;

	cx_bits_t owner;
	uint16_t cooldown;
	uint8_t owner_handle;
	uint8_t state;
};

struct sim_action *action_get(const struct simulation *sim, uint8_t id);
struct sim_action *action_add(struct simulation *sim, const struct action *act);
void action_del(struct simulation *sim, uint8_t id);
void sim_actions_init(struct simulation *sim);
void actions_flush(struct simulation *sim);
void action_complete(struct simulation *sim, uint8_t id);
enum iteration_result action_process(void *_sim, void *_sa);
#endif
