#include <stdio.h>

#include "world.h"
#include "update.h"
#include "serialize.h"

int main(int argc, const char **argv)
{
	struct ent e;
	struct update *eu1, *eu2;
	struct ent_update *eu;
	char buf[255];
	size_t b;

	ent_init(&e);
	e.id = 123;
	e.pos.x = -12345;
	e.pos.y = 54321;
	eu1 = ent_update_init(&e);
	eu2 = ent_update_init(NULL);

	b = pack_update(eu1, buf);
	b += pack_ent_update(eu1->update, &buf[b]);
	printf("packed %d bytes\n", b);
	b = unpack_update(eu2, buf);
	b += unpack_ent_update(eu2->update, &buf[b]);
	printf("unpacked %d bytes\n", b);

	eu = eu1->update;
	printf("> %d | %d | %d | %d\n", eu1->type, eu->id, eu->pos.x, eu->pos.y);
	eu = eu2->update;
	printf("< %d | %d | %d | %d\n", eu2->type, eu->id, eu->pos.x, eu->pos.y);

	return 0;
}
