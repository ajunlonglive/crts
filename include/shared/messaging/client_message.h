#ifndef __CLIENT_MESSAGE_H
#define __CLIENT_MESSAGE_H
#include <stdint.h>

#include "shared/math/geom.h"
#include "shared/sim/action.h"

enum client_message_type {
	client_message_poke,
	client_message_action,
	client_message_chunk_req,
};

struct cm_chunk_req {
	struct point pos;
};

struct cm_action {
	struct circle range;
	enum action_type type;
	uint16_t tgt;
	uint8_t workers;
};

struct client_message {
	enum client_message_type type;
	uint32_t client_id;

	union {
		struct cm_action action;
		struct cm_chunk_req chunk_req;
	} msg;
};

void cm_init(struct client_message *, enum client_message_type t, const void *src);
#endif
