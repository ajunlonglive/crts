#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/messaging/client_message.h"
#include "shared/util/log.h"

static void
cm_create_chunk_req(struct cm_chunk_req *cr, const struct point *p)
{
	cr->pos = *p;
}

static void
cm_create_action(struct cm_action *au, const struct action *a)
{
	memset(au, 0, sizeof(struct cm_action));

	if (a != NULL) {
		au->id = a->id;
		au->tgt = a->tgt;
		au->type = a->type;
		au->flags = a->flags;
		au->range = a->range;
		au->source = a->source;
		au->workers = a->workers_requested;
	}

	L("setting harvest target, (%d, %d), %dx%d",
		au->range.pos.x,
		au->range.pos.y,
		au->range.height,
		au->range.width
		);
}

void
cm_init(struct client_message *cm, enum client_message_type t, const void *src)
{
	cm->type = t;

	switch (t) {
	case client_message_poke:
		break;
	case client_message_action:
		cm_create_action(&cm->msg.action, src);
		break;
	case client_message_chunk_req:
		cm_create_chunk_req(&cm->msg.chunk_req, src);
		break;
	}
}
