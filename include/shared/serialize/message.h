#ifndef SHARED_SERIALIZE_MESSAGE_H
#define SHARED_SERIALIZE_MESSAGE_H

#include <stdint.h>

#include "shared/serialize/chunk.h"
#include "shared/serialize/coder.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"

enum message_type {
	mt_poke,
	mt_req,
	mt_ent,
	mt_tile,
	mt_chunk,
	mt_cursor,
	mt_connect,
	message_type_count
};

enum req_message_type {
	rmt_chunk,
	req_message_type_count,
};

struct msg_req {
	enum req_message_type mt;
	union {
		struct point chunk;
	} dat;
};

enum ent_message_type {
	emt_spawn,
	emt_kill,
	emt_update,
	ent_message_type_count,
};

struct msg_ent {
	enum ent_message_type mt;
	// TODO everywhere ent_id is 32 bits so this will break if used
	// directly
	uint16_t id;
	union {
		struct {
			struct point pos;
			uint16_t alignment;
			uint8_t modified;
		} update;
		struct {
			enum ent_type type;
			uint16_t alignment;
			struct point pos;
		} spawn;
	} dat;
};

struct msg_tile {
	struct point cp;
	uint8_t c;
	float height;
	uint8_t t;
};

struct msg_chunk {
	struct ser_chunk dat;
};

struct msg_cursor {
	struct point cursor;
	enum action action;
};

/* none can be above uint8_t max */
enum message_batch_size {
	mbs_req = 107,
	mbs_ent = 53,
	mbs_tile = 64,
	mbs_chunk = 1,
	mbs_cursor = 1,
};

struct message {
	union {
		struct msg_req req[mbs_req];
		struct msg_ent ent[mbs_ent];
		struct msg_tile tile[mbs_tile];
		struct msg_chunk chunk[mbs_chunk];
		struct msg_cursor cursor[mbs_cursor];
	} dat;
	enum message_type mt;
	uint8_t count;
};

_Static_assert(sizeof(struct message) <= sizeof(struct msg_chunk) + sizeof(enum message_type) + 4, "message batch size too big");

typedef void ((*msg_cb)(void *ctx, enum message_type, void *msg));

size_t pack_message(const struct message *msg, uint8_t *buf, uint32_t blen);
size_t unpack_message(uint8_t *buf, uint32_t blen, msg_cb cb, void *ctx);
bool append_msg(struct message *msg, void *smsg);
const char *inspect_message(enum message_type mt, const void *msg);
#endif
