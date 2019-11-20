#ifndef __WORLD_H
#define __WORLD_H
#include <stdlib.h>

struct world {
	size_t ecnt;
	size_t ecap;
	struct ent **ents;
};

struct ent {
	int x;
	int y;
	char c;
};
struct world *world_init(void);
void world_spawn(struct world *w, struct ent *e);
struct ent *ent_init(int x, int y, char c);
#endif
