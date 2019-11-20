#ifndef __WORLD_H
#define __WORLD_H
#include "alignment.h"
#include <stdlib.h>

struct world {
	size_t ecnt;
	size_t ecap;
	struct ent *ents;
};

struct ent {
	int x;
	int y;
	char c;
	struct alignment *alignment;
	int satisfaction;
	int age;
};

struct world *world_init(void);
struct ent *world_spawn(struct world *w);
#endif
