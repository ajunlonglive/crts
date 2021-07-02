#ifndef SERVER_SIM_ENT_H
#define SERVER_SIM_ENT_H

#include "server/sim/sim.h"
#include "shared/sim/world.h"

void kill_ent(struct simulation *sim, struct ent *e);
void destroy_ent(struct world *w, struct ent *e);
struct ent *spawn_ent(struct world *sim);
void damage_ent(struct simulation *sim, struct ent *e, uint8_t damage);
void simulate_ents(struct simulation *sim);
enum iteration_result process_spawn_iterator(void *_s, void *_e);
bool ent_pgraph_set(struct chunks *cnks, struct ent *e, const struct point *g);
enum result ent_pathfind(struct chunks *cnks, struct ent *e);
#endif
