#include "world.h"
#include "serialize.h"

int pack_ent(struct ent *e, char *buf)
{
	buf[0] = e->pos.x;
	buf[1] = e->pos.y;
	buf[2] = e->c;
	return 1;
}

int unpack_ent(struct ent *e, const char *buf)
{
	e->pos.x = buf[0];
	e->pos.y = buf[1];
	e->c = buf[2];
	return 1;
}

int pack_action(struct action *a, char *buf)
{
	buf[0] = a->type;
	return 1;
}

int unpack_action(struct action *a, const char *buf)
{
	a->type = buf[0];
	a->motivator = -1;
	a->id = -1;
	return 1;
}
