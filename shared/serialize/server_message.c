#include "messaging/server_message.h"
#include "serialize/base.h"
#include "serialize/geom.h"
#include "serialize/server_message.h"
#include <string.h>

size_t unpack_sm_ent(struct sm_ent *eu, const char *buf)
{
	size_t b = 0;

	b += unpack_int(&eu->id, buf);
	b += unpack_point(&eu->pos, &buf[b]);
	b += unpack_int(&eu->alignment, &buf[b]);

	return b;
}

size_t pack_sm_ent(const struct sm_ent *eu, char *buf)
{
	size_t b = 0;

	b += pack_int(&eu->id, buf);
	b += pack_point(&eu->pos, &buf[b]);
	b += pack_int(&eu->alignment, &buf[b]);

	return b;
}

size_t pack_cm_chunk(const struct sm_chunk *eu, char *buf)
{
	memcpy(buf, &eu->chunk, sizeof(struct chunk));
	return sizeof(struct chunk);
};

size_t unpack_cm_chunk(struct sm_chunk *eu, const char *buf)
{
	memcpy(&eu->chunk, buf, sizeof(struct chunk));
	return sizeof(struct chunk);
};
