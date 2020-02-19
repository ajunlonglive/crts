#include <stdlib.h>
#include <string.h>

#include "shared/messaging/client_message.h"

static struct cm_poke poke = {};

struct cm_poke *
cm_create_poke(void)
{
	return &poke;
}

struct cm_chunk_req *
cm_create_chunk_req(const struct point *p)
{
	struct cm_chunk_req *cr = malloc(sizeof(struct cm_chunk_req));

	memset(cr, 0, sizeof(struct cm_chunk_req));

	cr->pos = *p;

	return cr;
}

struct cm_action *
cm_create_action(const struct action *a)
{
	struct cm_action *au = malloc(sizeof(struct cm_action));

	memset(au, 0, sizeof(struct cm_action));

	if (a != NULL) {
		au->type = a->type;
		au->range = a->range;
		au->workers = a->workers_requested;
	}

	return au;
}

struct client_message *
cm_create(enum client_message_type t, void *src)
{
	void *payload;
	struct client_message *cm;

	cm = malloc(sizeof(struct client_message));

	switch (t) {
	case client_message_poke:
		payload = cm_create_poke();
		break;
	case client_message_action:
		payload = cm_create_action(src);
		break;
	case client_message_chunk_req:
		payload = cm_create_chunk_req(src);
		break;
	}

	cm->type = t;
	cm->update = payload;
	return cm;
}

void
cm_destroy(struct client_message *ud)
{
	switch (ud->type) {
	case client_message_poke:
		break;
	case client_message_action:
	case client_message_chunk_req:
		if (ud->update != NULL) {
			free(ud->update);
		}

		free(ud);
		break;
	}
}
