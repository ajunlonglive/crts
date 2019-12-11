#ifndef __PATHFIND_H
#define __PATHFIND_H
#include "types/geom.h"
#include "sim/world.h"

void pathfind(struct hash *chunks, struct point *s, struct point *f);
void meander(struct hash *chunks, struct point *s);
#endif
