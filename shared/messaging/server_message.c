#include <string.h>

#include "shared/messaging/server_message.h"
#include "shared/sim/action.h"
#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

static struct sm_ent *
sm_create_ent(const struct ent *e)
{
	struct sm_ent *eu = calloc(1, sizeof(struct sm_ent));

	if (e != NULL) {
		eu->id = e->id;
		eu->type = e->type;
		eu->pos = e->pos;
		eu->alignment = e->alignment->max;
	}

	return eu;
}

struct sm_chunk *
sm_create_chunk(const struct chunk *c)
{
	struct sm_chunk *cu = malloc(sizeof(struct sm_chunk));

	memset(cu, 0, sizeof(struct sm_chunk));

	cu->chunk = *c;

	return cu;
}

struct sm_action *
sm_create_action(const struct action *a)
{
	struct sm_action *au = malloc(sizeof(struct sm_action));

	memset(au, 0, sizeof(struct sm_action));

	au->action = *a;

	return au;
}

static struct sm_rem_action *
sm_create_rem_action(const long *id)
{
	struct sm_rem_action *ra = malloc(sizeof(struct sm_rem_action));

	//memset(ra, 0, sizeof(struct sm_rem_action));

	ra->id = *id;

	return ra;
}

struct server_message *
sm_create(enum server_message_type t, const void *src)
{
	void *payload;
	struct server_message *sm;

	sm = malloc(sizeof(struct server_message));

	switch (t) {
	case server_message_ent:
		payload = sm_create_ent(src);
		break;
	case server_message_chunk:
		payload = sm_create_chunk(src);
		break;
	case server_message_action:
		payload = sm_create_action(src);
		break;
	case server_message_rem_action:
		payload = sm_create_rem_action(src);
		break;
	}

	sm->type = t;
	sm->update = payload;
	return sm;
}

void
sm_destroy(struct server_message *ud)
{
	switch (ud->type) {
	case server_message_ent:
	case server_message_chunk:
	case server_message_action:
	case server_message_rem_action:
		if (ud->update != NULL) {
			free(ud->update);
		}

		free(ud);
		break;
	}
}
