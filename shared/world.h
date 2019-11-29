#ifndef __WORLD_H
#define __WORLD_H
#include "geom.h"
#include "alignment.h"
#include <stdlib.h>

struct world {
	size_t ecnt;
	size_t ecap;
	struct ent *ents;
	struct {
		size_t len;
		size_t cap;
		struct chunk *chunks;
	} chunks;
};

struct ent {
	int id;
	struct point pos;
	char c;
	struct alignment *alignment;
	int satisfaction;
	int age;

	int task;
	int idle;
};

#include "update.h"

struct world *world_init(void);
struct ent *world_spawn(struct world *w);
void world_despawn(struct world *w, int i);
void ent_init(struct ent *e);
void world_apply_update(struct world *w, struct update *ud);
#endif
