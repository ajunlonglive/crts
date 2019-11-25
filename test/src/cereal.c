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

	ent_init(&e);
	e.id = 123;
	e.pos.x = -12345;
	e.pos.y = 54321;
	eu1 = ent_update_init(&e);
	eu2 = ent_update_init(NULL);

	printf("packed %d bytes\n", pack_update(eu1, buf));
	printf("unpacked %d bytes\n", unpack_update(eu2, buf));

	eu = eu2->update;

	printf("update type: %d, for %d, (%d, %d)\n", eu2->type, eu->id, eu->pos.x, eu->pos.y);

	return 0;
}
