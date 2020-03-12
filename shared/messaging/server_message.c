#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <stdlib.h>
#include <string.h>

#include "shared/messaging/server_message.h"
#include "shared/sim/action.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

static void
sm_create_ent(struct sm_ent *msg, const struct ent *e)
{
	if (e != NULL) {
		msg->id = e->id;
		msg->type = e->type;
		msg->pos = e->pos;
		msg->alignment = e->alignment->max;
	}
}

static void
sm_create_chunk(struct sm_chunk *msg, const struct chunk *c)
{
	msg->chunk = *c;
}

static void
sm_create_action(struct sm_action *msg, const struct action *a)
{
	msg->action = *a;
}

static void
sm_create_rem_action(struct sm_rem_action *msg, const long *id)
{
	msg->id = *id;
}

static void
sm_create_world_info(struct sm_world_info *msg, const struct world *w)
{
	msg->ents = hdarr_len(w->ents);
}

static void
sm_create_hello(struct sm_hello *msg, const uint8_t *al)
{
	msg->alignment = *al;
}

static void
sm_create_kill_ent(struct sm_kill_ent *msg, const uint16_t *val)
{
	msg->id = *val;
}

void
sm_init(struct server_message *sm, enum server_message_type t, const void *src)
{
	sm->type = t;

	switch (t) {
	case server_message_ent:
		sm_create_ent(&sm->msg.ent, src);
		break;
	case server_message_chunk:
		sm_create_chunk(&sm->msg.chunk, src);
		break;
	case server_message_action:
		sm_create_action(&sm->msg.action, src);
		break;
	case server_message_rem_action:
		sm_create_rem_action(&sm->msg.rem_action, src);
		break;
	case server_message_world_info:
		sm_create_world_info(&sm->msg.world_info, src);
		break;
	case server_message_hello:
		sm_create_hello(&sm->msg.hello, src);
		break;
	case server_message_kill_ent:
		sm_create_kill_ent(&sm->msg.kill_ent, src);
		break;
	}
}
