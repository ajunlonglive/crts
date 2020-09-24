#ifndef SHARED_SERIALIZE_CHUNK_H
#define SHARED_SERIALIZE_CHUNK_H

#include "shared/serialize/message.h"

struct serial_chunk {
	struct point cp;
	enum tile tiles[CHUNK_SIZE * CHUNK_SIZE];
	float heights[CHUNK_SIZE * CHUNK_SIZE];
};

void fill_msg_chunk(struct msg_chunk *msg, const struct chunk *ck);
#endif
