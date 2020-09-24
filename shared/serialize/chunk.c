#include "posix.h"

#include <string.h>

#include "shared/serialize/chunk.h"
#include "shared/serialize/message.h"

void
fill_msg_chunk(struct msg_chunk *msg, const struct chunk *ck)
{
	*msg = (struct msg_chunk){ .cp = ck->pos };

	memcpy(msg->heights, ck->heights,
		sizeof(float) * CHUNK_SIZE * CHUNK_SIZE);

	memcpy(msg->tiles, ck->tiles,
		sizeof(enum tile) * CHUNK_SIZE * CHUNK_SIZE);
}

void
pack_serial_chunk(struct serial_chunk *sck)
{

}
