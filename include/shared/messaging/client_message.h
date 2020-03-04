#ifndef __CLIENT_MESSAGE_H
#define __CLIENT_MESSAGE_H
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/action.h"

enum client_message_type {
	client_message_poke,
	client_message_action,
	client_message_chunk_req,
	client_message_ent_req,
};

struct client_message {
	enum client_message_type type;
	void *update;
};

struct cm_chunk_req {
	struct point pos;
};

struct cm_action {
	enum action_type type;
	struct circle range;
	uint8_t workers;
};

struct cm_ent_req {
	uint32_t id;
};

struct client_message *cm_create(enum client_message_type t, void *e);
void cm_destroy(struct client_message *ud);
#endif
