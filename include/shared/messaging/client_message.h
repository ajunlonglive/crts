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

struct cm_chunk_req {
	struct point pos;
};

struct cm_action {
	enum action_type type;
	struct circle range;
	uint8_t workers;
	uint16_t tgt;
};

struct cm_ent_req {
	uint32_t id;
};

struct client_message {
	enum client_message_type type;
	uint32_t client_id;

	union {
		struct cm_action action;
		struct cm_chunk_req chunk_req;
		struct cm_ent_req ent_req;
	} msg;
};

void cm_init(struct client_message *, enum client_message_type t, const void *src);
#endif
