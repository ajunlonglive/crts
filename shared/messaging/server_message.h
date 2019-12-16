#ifndef __UPDATE_H
#define __UPDATE_H
#include "math/geom.h"
#include "sim/action.h"
#include "sim/chunk.h"
#include "sim/world.h"

enum server_message_type {
	server_message_ent,
	server_message_chunk
};

struct server_message {
	enum server_message_type type;
	void *update;
};

struct sm_ent {
	int id;
	struct point pos;
	int alignment;
};

struct sm_chunk {
	struct chunk chunk;
};

void sm_destroy(struct server_message *ud);
struct server_message *sm_create(enum server_message_type t, const void *src);
#endif
