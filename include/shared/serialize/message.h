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
	mt_server_info,
	mt_server_cmd,
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
	ent_id_t id;
	union {
		struct {
			struct point pos;
			uint16_t z;
			uint8_t modified;
		} update;
		struct {
			enum ent_type type;
			uint16_t z;
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
	float cursor_z;
	enum action action;
	uint16_t action_arg;
	bool once;
};

struct msg_server_info {
	float fps;
};

enum server_cmd {
	server_cmd_pause,
	server_cmd_unpause,
	server_cmd_count,
};

struct msg_server_cmd {
	enum server_cmd cmd;
};

/* none can be above uint8_t max */
enum message_batch_size {
	mbs_req = 107,
	mbs_ent = 53,
	mbs_tile = 64,
	mbs_chunk = 1,
	mbs_cursor = 64,
	mbs_server_info = 99,
	mbs_server_cmd = 99,
};

struct message {
	union {
		struct msg_req req[mbs_req];
		struct msg_ent ent[mbs_ent];
		struct msg_tile tile[mbs_tile];
		struct msg_chunk chunk[mbs_chunk];
		struct msg_cursor cursor[mbs_cursor];
		struct msg_server_info server_info[mbs_server_info];
		struct msg_server_cmd server_cmd[mbs_server_cmd];
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
