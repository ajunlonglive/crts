#include <string.h>

#include "messaging/server_message.h"
#include "sim/action.h"
#include "sim/alignment.h"
#include "sim/ent.h"

static struct sm_ent *sm_create_ent(const struct ent *e)
{
	struct sm_ent *eu = malloc(sizeof(struct sm_ent));

	memset(eu, 0, sizeof(struct sm_ent));

	if (e != NULL) {
		eu->id = e->id;
		eu->pos = e->pos;
		eu->alignment = e->alignment->max;
	}

	return eu;
}

struct sm_chunk *sm_create_chunk(const struct chunk *c)
{
	struct sm_chunk *cu = malloc(sizeof(struct sm_chunk));

	memset(cu, 0, sizeof(struct sm_chunk));

	cu->chunk = *c;

	return cu;
}

struct server_message *sm_create(enum server_message_type t, const void *src)
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
	}

	sm->type = t;
	sm->update = payload;
	return sm;
}

void sm_destroy(struct server_message *ud)
{
	switch (ud->type) {
	case server_message_ent:
	case server_message_chunk:
		if (ud->update != NULL)
			free(ud->update);

		free(ud);
		break;
	}
}
