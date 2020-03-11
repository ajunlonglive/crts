#include <string.h>

#include "shared/messaging/client_message.h"
#include "shared/serialize/base.h"
#include "shared/serialize/client_message.h"
#include "shared/serialize/geom.h"

size_t
unpack_cm_action(struct cm_action *au, const char *buf)
{
	size_t b = 0;

	b += unpack_int((int*)&au->type, &buf[b]);
	b += unpack_uint8_t((uint8_t *)&au->workers, &buf[b]);
	b += unpack_circle(&au->range, &buf[b]);
	b += unpack_uint16_t(&au->tgt, &buf[b]);

	return b;
}

size_t
pack_cm_action(const struct cm_action *au, char *buf)
{
	size_t b = 0;

	b += pack_int((int*)&au->type, &buf[b]);
	b += pack_uint8_t((uint8_t *)&au->workers, &buf[b]);
	b += pack_circle(&au->range, &buf[b]);
	b += pack_uint16_t(&au->tgt, &buf[b]);

	return b;
}

size_t
pack_cm_chunk_req(const struct cm_chunk_req *eu, char *buf)
{
	return pack_point(&eu->pos, buf);
};

size_t
unpack_cm_chunk_req(struct cm_chunk_req *eu, const char *buf)
{
	return unpack_point(&eu->pos, buf);
};

size_t
pack_cm_ent_req(const struct cm_ent_req *eu, char *buf)
{
	return pack_uint32_t(&eu->id, buf);
};

size_t
unpack_cm_ent_req(struct cm_ent_req *eu, const char *buf)
{
	return unpack_uint32_t(&eu->id, buf);
};

size_t
unpack_cm(struct client_message *ud, const char *buf)
{
	size_t b = 0;

	unpack_enum(client_message_type, &ud->type, &buf[b], b);
	b += unpack_uint32_t(&ud->client_id, &buf[b]);

	return b;
}

size_t
pack_cm(const struct client_message *ud, char *buf)
{
	size_t b = 0;

	pack_enum(client_message_type, &ud->type, buf, b);
	b += pack_uint32_t(&ud->client_id, &buf[b]);

	return b;
}
