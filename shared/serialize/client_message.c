#include <string.h>

#include "shared/messaging/client_message.h"
#include "shared/serialize/base.h"
#include "shared/serialize/client_message.h"
#include "shared/serialize/geom.h"
#include "shared/util/log.h"

static size_t
unpack_cm_action(struct cm_action *au, const char *buf)
{
	size_t b = 0;

	b += unpack_int((int*)&au->type, &buf[b]);
	b += unpack_uint8_t((uint8_t *)&au->workers, &buf[b]);
	b += unpack_circle(&au->range, &buf[b]);
	b += unpack_uint16_t(&au->tgt, &buf[b]);

	return b;
}

static size_t
pack_cm_action(const struct cm_action *au, char *buf)
{
	size_t b = 0;

	b += pack_int((int*)&au->type, &buf[b]);
	b += pack_uint8_t((uint8_t *)&au->workers, &buf[b]);
	b += pack_circle(&au->range, &buf[b]);
	b += pack_uint16_t(&au->tgt, &buf[b]);

	return b;
}

static size_t
pack_cm_chunk_req(const struct cm_chunk_req *eu, char *buf)
{
	return pack_point(&eu->pos, buf);
};

static size_t
unpack_cm_chunk_req(struct cm_chunk_req *eu, const char *buf)
{
	return unpack_point(&eu->pos, buf);
};

static size_t
pack_cm_ent_req(const struct cm_ent_req *eu, char *buf)
{
	return pack_uint32_t(&eu->id, buf);
};

static size_t
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

	switch (ud->type) {
	case client_message_poke:
		break;
	case client_message_action:
		b += unpack_cm_action(&ud->msg.action, &buf[b]);
		break;
	case client_message_chunk_req:
		b += unpack_cm_chunk_req(&ud->msg.chunk_req, &buf[b]);
		break;
	case client_message_ent_req:
		b += unpack_cm_ent_req(&ud->msg.ent_req, &buf[b]);
		break;
	}

	return b;
}

size_t
pack_cm(const void *cm, char *buf)
{
	size_t b = 0;
	const struct client_message *ud = cm;

	pack_enum(client_message_type, &ud->type, buf, b);
	b += pack_uint32_t(&ud->client_id, &buf[b]);

	switch (ud->type) {
	case client_message_poke:
		break;
	case client_message_action:
		b += pack_cm_action(&ud->msg.action, &buf[b]);
		break;
	case client_message_chunk_req:
		b += pack_cm_chunk_req(&ud->msg.chunk_req, &buf[b]);
		break;
	case client_message_ent_req:
		b += pack_cm_ent_req(&ud->msg.ent_req, &buf[b]);
		break;
	}

	return b;
}
