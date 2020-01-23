#include <stdio.h>

#include "sim/ent.h"
#include "messaging/server_message.h"
#include "serialize/server_message.h"
#include "messaging/client_message.h"
#include "serialize/client_message.h"

int
main(int argc, const char **argv)
{
	struct ent e;
	struct server_message *eu1, *eu2;
	struct sm_ent *eu;
	char buf[255];
	size_t b;

	ent_init(&e);
	e.id = 123;
	e.pos.x = -12345;
	e.pos.y = 54321;
	eu1 = sm_create(server_message_ent, &e);
	eu2 = sm_create(server_message_ent, NULL);

	b = pack_sm(eu1, buf);
	b += pack_sm_ent(eu1->update, &buf[b]);
	printf("packed %ld bytes\n", (long)b);
	b = unpack_sm(eu2, buf);
	b += unpack_sm_ent(eu2->update, &buf[b]);
	printf("unpacked %ld bytes\n", (long)b);

	eu = eu1->update;
	printf("> %d | %d | %d | %d\n", eu1->type, eu->id, eu->pos.x, eu->pos.y);
	eu = eu2->update;
	printf("< %d | %d | %d | %d\n", eu2->type, eu->id, eu->pos.x, eu->pos.y);

	struct client_message *cm1, *cm2;
	struct action move = {
		.type = at_move,
		.motivator = 0,
		.id = 0,
		.workers = 0,
		.workers_in_range = 0,
		.completion = 0,
		.range = {
			.center = { .x = 6, .y = 9 },
			.r = 1
		}
	};

	action_inspect(&move);

	cm1 = cm_create(client_message_action, &move);
	b = pack_cm(cm1, buf);
	b += pack_cm_action(cm1->update, &buf[b]);
	printf("packed %ld bytes\n", (long)b);
	struct cm_action *cma = cm1->update;
	printf("cm_act: type: %d, (%d, %d), %d\n", cma->type, cma->range.center.x, cma->range.center.y, cma->range.r);

	cm2 = cm_create(client_message_action, NULL);
	b = unpack_cm(cm2, buf);
	b += unpack_cm_action(cm2->update, &buf[b]);
	printf("unpacked %ld bytes\n", (long)b);
	cma = cm2->update;
	printf("cm_act: type: %d, (%d, %d), %d\n", cma->type, cma->range.center.x, cma->range.center.y, cma->range.r);

	return 0;
}
