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

static int unpack_point(struct point *p, const char *buf)
{
	p->x = buf[0];
	p->y = buf[1];

	return 1;
}

static int pack_point(const struct point *p, char *buf)
{
	buf[0] = p->x;
	buf[1] = p->y;

	return 1;
}

static int unpack_ent_update(struct ent_update *eu, const char *buf)
{
	eu->id = buf[0];
	unpack_point(&eu->pos, &buf[1]);

	return 1;
}

static int pack_ent_update(const struct ent_update *eu, char *buf)
{
	buf[0] = eu->id;
	pack_point(&eu->pos, &buf[1]);

	return 1;
}

int pack_update(const struct update *ud, char *buf)
{
	buf[0] = ud->type;
	switch (ud->type) {
	case update_type_ent:
		pack_ent_update(ud->update, &buf[1]);
	}

	return 1;
}

int unpack_update(struct update *ud, const char *buf)
{
	ud->type = buf[0];
	switch (ud->type) {
	case update_type_ent:
		unpack_ent_update(ud->update, &buf[1]);
	}

	return 1;
}
