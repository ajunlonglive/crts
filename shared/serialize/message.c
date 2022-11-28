#include "posix.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "shared/serialize/base.h"
#include "shared/serialize/coder.h"
#include "shared/serialize/limits.h"
#include "shared/serialize/message.h"
#include "shared/sim/action.h"
#include "shared/sim/chunk.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

void
pack_msg_req(struct ac_coder *cod, const struct msg_req *msg)
{
	cod->lim = req_message_type_count;
	ac_pack(cod, msg->mt);

	switch (msg->mt) {
	case rmt_chunk:
		pack_point(cod, &msg->dat.chunk, MAX_COORD, 0, CHUNK_SIZE);
		break;
	default:
		assert(false);
	}
}

void
unpack_msg_req(struct ac_decoder *dec, struct msg_req *msg)
{
	dec->lim = req_message_type_count;
	uint32_t mt;
	ac_unpack(dec, &mt, 1);
	msg->mt = mt;

	switch (msg->mt) {
	case rmt_chunk:
		unpack_point(dec, &msg->dat.chunk, MAX_COORD, 0, CHUNK_SIZE);
		break;
	default:
		assert(false);
	}
}

void
pack_msg_ent(struct ac_coder *cod, const struct msg_ent *msg)
{
	cod->lim = ent_message_type_count;
	ac_pack(cod, msg->mt);

	cod->lim = UINT16_MAX;
	ac_pack(cod, msg->id & 0xffff);
	ac_pack(cod, msg->id >> 16);

	switch (msg->mt) {
	case emt_spawn:
		cod->lim = ent_type_count;
		ac_pack(cod, msg->dat.spawn.type);

		pack_point(cod, &msg->dat.spawn.pos, MAX_COORD, 0, 1);

		cod->lim = UINT16_MAX;
		ac_pack(cod, msg->dat.spawn.z);
		break;
	case emt_kill:
		break;
	case emt_update:
		cod->lim = ent_update_type_max;
		ac_pack(cod, msg->dat.update.modified);

		if (msg->dat.update.modified & eu_pos) {
			pack_point(cod, &msg->dat.update.pos, MAX_COORD, 0, 1);
			cod->lim = UINT16_MAX;
			ac_pack(cod, msg->dat.update.z);
		}
		break;
	default:
		assert(false);
	}
}

void
unpack_msg_ent(struct ac_decoder *dec, struct msg_ent *msg)
{
	uint32_t v, va[2];

	dec->lim = ent_message_type_count;
	ac_unpack(dec, &v, 1);
	msg->mt = v;

	dec->lim = UINT16_MAX;
	ac_unpack(dec, va, 2);
	msg->id = va[0] | (va[1] << 16);

	switch (msg->mt) {
	case emt_spawn:
		dec->lim = ent_type_count;
		ac_unpack(dec, &v, 1);
		msg->dat.spawn.type = v;

		unpack_point(dec, &msg->dat.spawn.pos, MAX_COORD, 0, 1);

		dec->lim = UINT16_MAX;
		ac_unpack(dec, &v, 1);
		msg->dat.spawn.z = v;
		break;
	case emt_kill:
		break;
	case emt_update:
		dec->lim = ent_update_type_max;
		ac_unpack(dec, &v, 1);
		msg->dat.update.modified = v;

		if (msg->dat.update.modified & eu_pos) {
			unpack_point(dec, &msg->dat.update.pos, MAX_COORD, 0, 1);
			dec->lim = UINT16_MAX;
			ac_unpack(dec, &v, 1);
			msg->dat.update.z = v;
		}
		break;
	default:
		assert(false);
	}
}

void
pack_msg_tile(struct ac_coder *cod, const struct msg_tile *msg)
{
	pack_point(cod, &msg->cp, MAX_COORD, 0, CHUNK_SIZE);

	cod->lim = UINT8_MAX;
	ac_pack(cod, msg->c);

	cod->lim = STEPS;
	ac_pack(cod, quantizef(msg->height,
		MIN_HEIGHT, MAX_HEIGHT, STEPS));

	cod->lim = tile_count;
	ac_pack(cod, msg->t);
}

void
unpack_msg_tile(struct ac_decoder *dec, struct msg_tile *msg)
{
	unpack_point(dec, &msg->cp, MAX_COORD, 0, CHUNK_SIZE);

	uint32_t v;

	dec->lim = UINT8_MAX;
	ac_unpack(dec, &v, 1);
	msg->c = v;

	dec->lim = STEPS;
	ac_unpack(dec, &v, 1);
	msg->height = unquantizef(v,
		MIN_HEIGHT, MAX_HEIGHT, STEPS);

	dec->lim = tile_count;
	ac_unpack(dec, &v, 1);
	msg->t = v;
}

static void
pack_msg_chunk(struct ac_coder *cod, const struct msg_chunk *msg)
{
	pack_ser_chunk(cod, &msg->dat);
}

static void
unpack_msg_chunk(struct ac_decoder *dec, struct msg_chunk *msg)
{
	unpack_ser_chunk(dec, &msg->dat);
}

static void
pack_msg_cursor(struct ac_coder *cod, const struct msg_cursor *msg)
{
	pack_point(cod, &msg->cursor, MAX_COORD, 0, 1);
	cod->lim = action_count;
	ac_pack(cod, msg->action);
	cod->lim = UINT16_MAX;
	ac_pack(cod, msg->action_arg);
	cod->lim = STEPS;
	ac_pack(cod, quantizef(msg->cursor_z, MIN_HEIGHT, MAX_HEIGHT, cod->lim));
	cod->lim = 2;
	ac_pack(cod, msg->once);
}

static void
unpack_msg_cursor(struct ac_decoder *dec, struct msg_cursor *msg)
{
	unpack_point(dec, &msg->cursor, MAX_COORD, 0, 1);
	dec->lim = action_count;
	uint32_t v;
	ac_unpack(dec, &v, 1);
	msg->action = v;
	dec->lim = UINT16_MAX;
	ac_unpack(dec, &v, 1);
	msg->action_arg = v;
	dec->lim = STEPS;
	ac_unpack(dec, &v, 1);
	msg->cursor_z = unquantizef(v, MIN_HEIGHT, MAX_HEIGHT, dec->lim);
	dec->lim = 2;
	ac_unpack(dec, &v, 1);
	msg->once = v;
}

static void
pack_msg_server_info(struct ac_coder *cod, const struct msg_server_info *msg)
{
	cod->lim = 1024;
	ac_pack(cod, quantizef(msg->fps, 0.0f, 1000.0f, cod->lim));
}

static void
unpack_msg_server_info(struct ac_decoder *dec, struct msg_server_info *msg)
{
	uint32_t v;
	dec->lim = 1024;
	ac_unpack(dec, &v, 1);
	msg->fps = unquantizef(v, 0.0f, 1000.0f, dec->lim);
}

static void
pack_msg_server_cmd(struct ac_coder *cod, const struct msg_server_cmd *msg)
{
	cod->lim = server_cmd_count;
	ac_pack(cod, msg->cmd);
}

static void
unpack_msg_server_cmd(struct ac_decoder *dec, struct msg_server_cmd *msg)
{
	dec->lim = server_cmd_count;
	uint32_t v;
	ac_unpack(dec, &v, 1);
	msg->cmd = v;
}

size_t
pack_message(const struct message *msg, uint8_t *buf, uint32_t blen)
{
	uint32_t i;
	struct ac_coder cod;
	ac_pack_init(&cod, buf, blen);

	cod.lim = message_type_count;
	ac_pack(&cod, msg->mt);

	cod.lim = UINT8_MAX;
	ac_pack(&cod, msg->count);

	switch (msg->mt) {
	case mt_poke:
		break;
	case mt_req:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_req(&cod, &msg->dat.req[i]);
		}
		break;
	case mt_ent:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_ent(&cod, &msg->dat.ent[i]);
		}
		break;
	case mt_tile:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_tile(&cod, &msg->dat.tile[i]);
		}
		break;
	case mt_chunk:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_chunk(&cod, &msg->dat.chunk[i]);
		}
		break;
	case mt_cursor:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_cursor(&cod, &msg->dat.cursor[i]);
		}
		break;
	case mt_server_info:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_server_info(&cod, &msg->dat.server_info[i]);
		}
		break;
	case mt_server_cmd:
		for (i = 0; i < msg->count; ++i) {
			pack_msg_server_cmd(&cod, &msg->dat.server_cmd[i]);
		}
		break;
	case mt_connect:
	case message_type_count:
		assert(false);
	}

	ac_pack_finish(&cod);

	/* L(log_misc, "packed    %p@%ld", (void *)buf, ac_coder_len(&cod)); */
	return ac_coder_len(&cod);
}

size_t
unpack_message(uint8_t *buf, uint32_t blen, msg_cb cb, void *ctx)
{
	/* L(log_misc, "unpacking %p@%d", (void *)buf, blen); */
	uint32_t v, i;
	enum message_type mt;
	uint8_t cnt;

	struct ac_decoder dec;
	ac_unpack_init(&dec, buf, blen);

	dec.lim = message_type_count;
	ac_unpack(&dec, &v, 1);
	mt = v;

	dec.lim = UINT8_MAX;
	ac_unpack(&dec, &v, 1);
	cnt = v;

	switch (mt) {
	case mt_poke:
		break;
	case mt_req:
		for (i = 0; i < cnt; ++i) {
			struct msg_req m = { 0 };
			unpack_msg_req(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_ent:
		for (i = 0; i < cnt; ++i) {
			struct msg_ent m = { 0 };
			unpack_msg_ent(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_tile:
		for (i = 0; i < cnt; ++i) {
			struct msg_tile m = { 0 };
			unpack_msg_tile(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_chunk:
		for (i = 0; i < cnt; ++i) {
			struct msg_chunk m = { 0 };
			unpack_msg_chunk(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_cursor:
		for (i = 0; i < cnt; ++i) {
			struct msg_cursor m = { 0 };
			unpack_msg_cursor(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_server_info:
		for (i = 0; i < cnt; ++i) {
			struct msg_server_info m = { 0 };
			unpack_msg_server_info(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_server_cmd:
		for (i = 0; i < cnt; ++i) {
			struct msg_server_cmd m = { 0 };
			unpack_msg_server_cmd(&dec, &m);
			cb(ctx, mt, &m);
		}
		break;
	case mt_connect:
	case message_type_count:
		assert(false);
	}

	return ac_decoder_len(&dec);
}

bool
append_msg(struct message *msg, void *smsg)
{
	void *dest = NULL;
	size_t len = 0;
	uint32_t lim = 0;

	switch (msg->mt) {
	case mt_poke:
		return false;
	case mt_req:
		lim = mbs_req;
		dest = &msg->dat.req[msg->count];
		len = sizeof(struct msg_req);
		break;
	case mt_ent:
		lim  = mbs_ent;
		dest = &msg->dat.ent[msg->count];
		len = sizeof(struct msg_ent);
		break;
	case mt_tile:
		lim =  mbs_tile;
		dest = &msg->dat.tile[msg->count];
		len = sizeof(struct msg_tile);
		break;
	case mt_chunk:
		lim = mbs_chunk;
		dest = &msg->dat.chunk[msg->count];
		len = sizeof(struct msg_chunk);
		break;
	case mt_cursor:
		lim = mbs_cursor;
		dest = &msg->dat.cursor[msg->count];
		len = sizeof(struct msg_cursor);
		break;
	case mt_server_info:
		lim = mbs_server_info;
		dest = &msg->dat.server_info[msg->count];
		len = sizeof(struct msg_server_info);
		break;
	case mt_server_cmd:
		lim = mbs_server_cmd;
		dest = &msg->dat.server_cmd[msg->count];
		len = sizeof(struct msg_server_cmd);
		break;
	case mt_connect:
	case message_type_count:
		assert(false);
		return false;
	}

	if (msg->count >= lim) {
		return false;
	}

	memcpy(dest, smsg, len);
	++msg->count;

	return true;
}

#define BS 256

const char *
inspect_message(enum message_type mt, const void *msg)
{
	static char str[BS] = { 0 };

	switch (mt) {
	case mt_poke:
		snprintf(str, BS, "poke:{}");
		break;
	case mt_req: {
		const struct msg_req *mr = msg;

		switch (mr->mt) {
		case rmt_chunk:
			snprintf(str, BS, "req:chunk:{%d, %d}",
				mr->dat.chunk.x, mr->dat.chunk.y);
			break;
		default:
			snprintf(str, BS, "req:?");
			break;
		}
		break;
	}
	case mt_ent: {
		const struct msg_ent *me = msg;

		uint32_t l = snprintf(str, BS, "ent:%d:", me->id);

		switch (me->mt) {
		case emt_spawn:
			snprintf(str + l, BS - l,
				"spawn:{pos: (%d, %d), type: %d}",
				me->dat.spawn.pos.x, me->dat.spawn.pos.y,
				me->dat.spawn.type);
			break;
		case emt_kill:
			snprintf(str + l, BS - l, "kill:{}");
			break;
		case emt_update:
			l += snprintf(str + l, BS - l, "update:");

			if (me->dat.update.modified & eu_pos) {
				l += snprintf(str + l, BS - l, "pos:(%d, %d) ",
					me->dat.update.pos.x, me->dat.update.pos.y);
			}
			break;
		default:
			snprintf(str, BS, "ent:?");
			break;
		}
		break;
		break;
	}
	case mt_tile: {
		const struct msg_tile *t = msg;
		snprintf(str, BS, "tile:{cp: (%d,%d)@%d, elev: %f, type: %d}",
			t->cp.x, t->cp.y, t->c, t->height, t->t);
		break;
	}
	case mt_chunk: {
		const struct msg_chunk *c = msg;
		snprintf(str, BS, "chunk:{cp: (%d,%d), ...}", c->dat.cp.x, c->dat.cp.y);
		break;
	}
	case mt_cursor: {
		const struct msg_cursor *c = msg;
		snprintf(str, BS, "cursor:{(%d,%d)}", c->cursor.x, c->cursor.y);
		break;
	}
	case mt_server_info: {
		const struct msg_server_info *info = msg;
		snprintf(str, BS, "server_info:{fps:%0.1f}", info->fps);
		break;
	}
	case mt_server_cmd: {
		const struct msg_server_cmd *cmd = msg;
		snprintf(str, BS, "server_info:{cmd:%d}", cmd->cmd);
		break;
	}
	default:
		snprintf(str, BS, "?");
		break;
	}

	return str;
}
