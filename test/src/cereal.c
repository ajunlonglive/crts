#include <stdio.h>

#include "sim/ent.h"
#include "messaging/server_message.h"
#include "serialize/server_message.h"

int main(int argc, const char **argv)
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
	printf("packed %d bytes\n", b);
	b = unpack_sm(eu2, buf);
	b += unpack_sm_ent(eu2->update, &buf[b]);
	printf("unpacked %d bytes\n", b);

	eu = eu1->update;
	printf("> %d | %d | %d | %d\n", eu1->type, eu->id, eu->pos.x, eu->pos.y);
	eu = eu2->update;
	printf("< %d | %d | %d | %d\n", eu2->type, eu->id, eu->pos.x, eu->pos.y);

	return 0;
}
