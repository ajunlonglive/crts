#ifndef __UPDATE_H
#define __UPDATE_H
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
#endif
