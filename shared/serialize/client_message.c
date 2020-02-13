#include "shared/messaging/client_message.h"
#include "shared/serialize/base.h"
#include "shared/serialize/client_message.h"
#include "shared/serialize/geom.h"

size_t
unpack_cm_action(struct cm_action *au, const char *buf)
{
	size_t b = 0;

	b += unpack_int((int*)&au->type, buf);
	b += unpack_circle(&au->range, &buf[b]);

	return b;
}

size_t
pack_cm_action(const struct cm_action *au, char *buf)
{
	size_t b = 0;

	b += pack_int((int*)&au->type, buf);
	b += pack_circle(&au->range, &buf[b]);

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
unpack_cm(struct client_message *ud, const char *buf)
{
	return unpack_int((int*)&ud->type, buf);
}

size_t
pack_cm(const struct client_message *ud, char *buf)
{
	return pack_int((int*)&ud->type, buf);
}

