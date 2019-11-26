#ifndef __UPDATE_H
#define __UPDATE_H
#include "geom.h"

enum update_type {
	update_type_ent,
	update_type_poke,
	update_type_action
};

struct update {
	enum update_type type;
	void *update;
};

struct ent_update {
	int id;
	struct point pos;
};

#include "world.h"
struct update *ent_update_init(struct ent *e);
struct update *poke_update_init(void);
void update_destroy(struct update *ud);
#endif
