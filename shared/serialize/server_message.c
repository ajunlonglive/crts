#include <string.h>

#include "shared/messaging/server_message.h"
#include "shared/serialize/base.h"
#include "shared/serialize/geom.h"
#include "shared/serialize/server_message.h"

size_t
unpack_sm_ent(struct sm_ent *eu, const char *buf)
{
	size_t b = 0;

	b += unpack_uint32_t(&eu->id, buf);
	unpack_enum(ent_type, &eu->type, &buf[b], b);
	b += unpack_point(&eu->pos, &buf[b]);
	b += unpack_uint8_t(&eu->alignment, &buf[b]);

	return b;
}

static size_t
pack_sm_ent(const struct sm_ent *eu, char *buf)
{
	size_t b = 0;

	b += pack_uint32_t(&eu->id, buf);
	pack_enum(ent_type, &eu->type, &buf[b], b);
	b += pack_point(&eu->pos, &buf[b]);
	b += pack_uint8_t(&eu->alignment, &buf[b]);

	return b;
}

static size_t
pack_sm_chunk(const struct sm_chunk *eu, char *buf)
{
	size_t b = 0, tiles = sizeof(enum tile) * CHUNK_SIZE * CHUNK_SIZE;

	b += pack_point(&eu->chunk.pos, &buf[b]);

	memcpy(&buf[b], &eu->chunk.tiles, tiles);
	b += tiles;

	b += pack_bool(&eu->chunk.empty, &buf[b]);

	return b;
};

size_t
unpack_sm_chunk(struct sm_chunk *eu, const char *buf)
{
	size_t b = 0, tiles = sizeof(enum tile) * CHUNK_SIZE * CHUNK_SIZE;

	b += unpack_point(&eu->chunk.pos, &buf[b]);

	memcpy(&eu->chunk.tiles, &buf[b], tiles);
	b += tiles;

	b += unpack_bool(&eu->chunk.empty, &buf[b]);

	return b;
};

static size_t
pack_sm_action(const struct sm_action *eu, char *buf)
{
	size_t b = 0;

	memcpy(&buf[b], &eu->action.type, sizeof(enum action_type));
	b += sizeof(enum action_type);

	b += pack_circle(&eu->action.range, &buf[b]);
	b += pack_uint8_t(&eu->action.workers_requested, &buf[b]);
	b += pack_uint8_t(&eu->action.id, &buf[b]);

	return b;
};

size_t
unpack_sm_action(struct sm_action *eu, const char *buf)
{
	size_t b = 0;

	memcpy(&eu->action.type, &buf[b], sizeof(enum action_type));
	b += sizeof(enum action_type);

	b += unpack_circle(&eu->action.range, &buf[b]);
	b += unpack_uint8_t(&eu->action.workers_requested, &buf[b]);
	b += unpack_uint8_t(&eu->action.id, &buf[b]);

	return b;
};

static size_t
pack_sm_rem_action(const struct sm_rem_action *eu, char *buf)
{
	return pack_long(&eu->id, buf);
};

size_t
unpack_sm_rem_action(struct sm_rem_action *eu, const char *buf)
{
	return unpack_long(&eu->id, buf);
};

static size_t
pack_sm_world_info(const struct sm_world_info *eu, char *buf)
{
	return pack_size_t(&eu->ents, buf);
};

size_t
unpack_sm_world_info(struct sm_world_info *eu, const char *buf)
{
	return unpack_size_t(&eu->ents, buf);
};

static size_t
pack_sm_kill_ent(const struct sm_kill_ent *eu, char *buf)
{
	return pack_uint32_t(&eu->id, buf);
};

size_t
unpack_sm_kill_ent(struct sm_kill_ent *eu, const char *buf)
{
	return unpack_uint32_t(&eu->id, buf);
};

static size_t
pack_sm_hello(const struct sm_hello *eu, char *buf)
{
	return pack_uint8_t(&eu->alignment, buf);
};

size_t
unpack_sm_hello(struct sm_hello *eu, const char *buf)
{
	return unpack_uint8_t(&eu->alignment, buf);
};

size_t
unpack_sm(struct server_message *ud, const char *buf)
{
	return unpack_int((int*)&ud->type, buf);
}

size_t
pack_sm(const struct server_message *sm, char *buf)
{
	size_t b = 0;

	pack_enum(server_message_type, &sm->type, buf, b);

	switch (sm->type) {
	case server_message_ent:
		b += pack_sm_ent(sm->update, &buf[b]);
		break;
	case server_message_chunk:
		b += pack_sm_chunk(sm->update, &buf[b]);
		break;
	case server_message_action:
		b += pack_sm_action(sm->update, &buf[b]);
		break;
	case server_message_rem_action:
		b += pack_sm_rem_action(sm->update, &buf[b]);
		break;
	case server_message_world_info:
		b += pack_sm_world_info(sm->update, &buf[b]);
		break;
	case server_message_hello:
		b += pack_sm_hello(sm->update, &buf[b]);
		break;
	case server_message_kill_ent:
		b += pack_sm_kill_ent(sm->update, &buf[b]);
		break;
	}

	return b;
}
