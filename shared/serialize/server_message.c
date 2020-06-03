#include <string.h>

#include "shared/messaging/server_message.h"
#include "shared/serialize/base.h"
#include "shared/serialize/geom.h"
#include "shared/serialize/server_message.h"

static size_t
unpack_sm_ent(struct sm_ent *eu, const char *buf)
{
	size_t i, b = 0;

	for (i = 0; i < SM_ENT_LEN; ++i) {
		b += unpack_uint8_t(&eu->updates[i].type, &buf[b]);
		b += unpack_uint8_t(&eu->updates[i].alignment, &buf[b]);
		b += unpack_uint8_t(&eu->updates[i].ent_type, &buf[b]);
		//b += unpack_uint8_t(&eu->updates[i]._, &buf[b]);

		if ((eu->updates[i].type) ==  eut_none) {
			return b;
		}

		b += unpack_uint32_t(&eu->updates[i].id, &buf[b]);

		switch (eu->updates[i].type) {
		case eut_kill:
			break;
		case eut_pos:
			b += unpack_point(&eu->updates[i].ud.pos, &buf[b]);
			break;
		}
	}

	return b;
}

static size_t
pack_sm_ent(const struct sm_ent *eu, char *buf)
{
	size_t i, b = 0;

	for (i = 0; i < SM_ENT_LEN; ++i) {
		b += pack_uint8_t(&eu->updates[i].type, &buf[b]);
		b += pack_uint8_t(&eu->updates[i].alignment, &buf[b]);
		b += pack_uint8_t(&eu->updates[i].ent_type, &buf[b]);
		//b += pack_uint8_t(&eu->updates[i]._, &buf[b]);

		if ((eu->updates[i].type) == eut_none) {
			return b;
		}

		b += pack_uint32_t(&eu->updates[i].id, &buf[b]);

		switch (eu->updates[i].type) {
		case eut_kill:
			break;
		case eut_pos:
			b += pack_point(&eu->updates[i].ud.pos, &buf[b]);
			break;
		}
	}

	return b;
}

static size_t
pack_sm_chunk(const struct sm_chunk *eu, char *buf)
{
	size_t b = 0, tiles =  CHUNK_SIZE * CHUNK_SIZE;

	b += pack_point(&eu->chunk.pos, &buf[b]);

	memcpy(&buf[b], &eu->chunk.tiles, sizeof(uint32_t) * tiles);
	b += sizeof(uint32_t) * tiles;

	memcpy(&buf[b], &eu->chunk.heights, sizeof(float) * tiles);
	b += sizeof(float) * tiles;

	b += pack_bool(&eu->chunk.empty, &buf[b]);

	return b;
}

static size_t
unpack_sm_chunk(struct sm_chunk *eu, const char *buf)
{
	size_t b = 0, tiles = CHUNK_SIZE * CHUNK_SIZE;

	b += unpack_point(&eu->chunk.pos, &buf[b]);

	memcpy(&eu->chunk.tiles, &buf[b], sizeof(uint32_t) * tiles);
	b += sizeof(uint32_t) * tiles;

	memcpy(&eu->chunk.heights, &buf[b], sizeof(float) * tiles);
	b += sizeof(float) * tiles;

	b += unpack_bool(&eu->chunk.empty, &buf[b]);

	return b;
}

static size_t
pack_sm_action(const struct sm_action *eu, char *buf)
{
	size_t b = 0;

	memcpy(&buf[b], &eu->action.type, sizeof(enum action_type));
	b += sizeof(enum action_type);

	b += pack_circle(&eu->action.range, &buf[b]);
	b += pack_uint16_t(&eu->action.workers_requested, &buf[b]);
	b += pack_uint8_t(&eu->action.id, &buf[b]);

	return b;
}

static size_t
unpack_sm_action(struct sm_action *eu, const char *buf)
{
	size_t b = 0;

	memcpy(&eu->action.type, &buf[b], sizeof(enum action_type));
	b += sizeof(enum action_type);

	b += unpack_circle(&eu->action.range, &buf[b]);
	b += unpack_uint16_t(&eu->action.workers_requested, &buf[b]);
	b += unpack_uint8_t(&eu->action.id, &buf[b]);

	return b;
}

static size_t
pack_sm_rem_action(const struct sm_rem_action *eu, char *buf)
{
	return pack_long(&eu->id, buf);
}

static size_t
unpack_sm_rem_action(struct sm_rem_action *eu, const char *buf)
{
	return unpack_long(&eu->id, buf);
}

static size_t
pack_sm_world_info(const struct sm_world_info *eu, char *buf)
{
	return pack_size_t(&eu->ents, buf);
}

static size_t
unpack_sm_world_info(struct sm_world_info *eu, const char *buf)
{
	return unpack_size_t(&eu->ents, buf);
}

static size_t
pack_sm_hello(const struct sm_hello *eu, char *buf)
{
	return pack_uint8_t(&eu->alignment, buf);
}

static size_t
unpack_sm_hello(struct sm_hello *eu, const char *buf)
{
	return unpack_uint8_t(&eu->alignment, buf);
}

size_t
unpack_sm(struct server_message *sm, const char *buf)
{
	size_t b = 0;

	unpack_enum(server_message_type, &sm->type, buf, b);

	switch (sm->type) {
	case server_message_ent:
		b += unpack_sm_ent(&sm->msg.ent, &buf[b]);
		break;
	case server_message_chunk:
		b += unpack_sm_chunk(&sm->msg.chunk, &buf[b]);
		break;
	case server_message_action:
		b += unpack_sm_action(&sm->msg.action, &buf[b]);
		break;
	case server_message_rem_action:
		b += unpack_sm_rem_action(&sm->msg.rem_action, &buf[b]);
		break;
	case server_message_world_info:
		b += unpack_sm_world_info(&sm->msg.world_info, &buf[b]);
		break;
	case server_message_hello:
		b += unpack_sm_hello(&sm->msg.hello, &buf[b]);
		break;
	}

	return b;
}

size_t
pack_sm(const void *vp, char *buf)
{
	size_t b = 0;
	const struct server_message *sm = vp;

	pack_enum(server_message_type, &sm->type, buf, b);

	switch (sm->type) {
	case server_message_ent:
		b += pack_sm_ent(&sm->msg.ent, &buf[b]);
		break;
	case server_message_chunk:
		b += pack_sm_chunk(&sm->msg.chunk, &buf[b]);
		break;
	case server_message_action:
		b += pack_sm_action(&sm->msg.action, &buf[b]);
		break;
	case server_message_rem_action:
		b += pack_sm_rem_action(&sm->msg.rem_action, &buf[b]);
		break;
	case server_message_world_info:
		b += pack_sm_world_info(&sm->msg.world_info, &buf[b]);
		break;
	case server_message_hello:
		b += pack_sm_hello(&sm->msg.hello, &buf[b]);
		break;
	}

	return b;
}
