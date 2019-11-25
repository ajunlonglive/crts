#ifndef __UPDATE_H
#define __UPDATE_H
#include "world.h"
#include "geom.h"

enum update_type {
	update_type_ent
};

struct update {
	enum update_type type;
	void *update;
};

struct ent_update {
	int id;
	struct point pos;
};

struct update *ent_update_init(struct ent *e);
void update_destroy(struct update *ud);
#endif
