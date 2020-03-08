#ifndef __UPDATE_H
#define __UPDATE_H

#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"

#ifdef CRTS_SERVER
#include "server/net/connection.h"
#endif

enum server_message_type {
	server_message_ent,
	server_message_chunk,
	server_message_action,
	server_message_rem_action,
	server_message_world_info,
	server_message_hello,
	server_message_kill_ent,
};

struct server_message {
	enum server_message_type type;
	void *update;
#ifdef CRTS_SERVER
	struct connection *dest;
#endif
};

struct sm_ent {
	uint32_t id;
	struct point pos;
	uint8_t alignment;
	enum ent_type type;
};

struct sm_kill_ent {
	uint32_t id;
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

void sm_destroy(struct server_message *ud);
struct server_message *sm_create(enum server_message_type t, const void *src);
#endif
