#ifndef __UPDATE_H
#define __UPDATE_H

#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"

enum server_message_type {
	server_message_ent,
	server_message_chunk,
	server_message_action,
	server_message_rem_action,
	server_message_world_info,
	server_message_hello,
};

enum ent_update_type {
	eut_none,
	eut_pos,
	eut_kill,
	eut_align,
};

#define SM_ENT_LEN 64
struct sm_ent {
	struct {
		uint32_t type;
		uint32_t id;
		union {
			struct point pos;
			uint8_t alignment;
		} ud;
	} updates[SM_ENT_LEN];
};

struct sm_chunk {
	struct chunk chunk;
};

struct sm_hello {
	uint8_t alignment;
};

struct sm_world_info {
	size_t ents;
};

struct sm_action {
	struct action action;
};

struct sm_rem_action {
	long id;
};

struct server_message {
	union {
		struct sm_ent ent;
		struct sm_chunk chunk;
		struct sm_hello hello;
		struct sm_world_info world_info;
		struct sm_action action;
		struct sm_rem_action rem_action;
	} msg;

	enum server_message_type type;
};

void sm_init(struct server_message *sm, enum server_message_type t, const void *src);
#endif
