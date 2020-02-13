#include <string.h>

#include "shared/serialize/base.h"
#include "shared/serialize/geom.h"
#include "shared/sim/action.h"
#include "shared/sim/ent.h"
#include "shared/sim/world.h"
#include "shared/util/log.h"

size_t
pack_ent(struct ent *e, char *buf)
{
	size_t b = 0;

	b += pack_point(&e->pos, buf);
	b += pack_char(&e->c, &buf[b]);

	return b;
}

size_t
unpack_ent(struct ent *e, const char *buf)
{
	size_t b = 0;

	b += unpack_point(&e->pos, buf);
	b += unpack_char(&e->c, &buf[b]);

	return b;
}

size_t
unpack_action(struct action *a, const char *buf)
{
	size_t b = 0;

	b += unpack_int((int*)&a->type, buf);
	a->motivator = -1;
	a->id = -1;

	return b;
}

size_t
pack_action(struct action *a, char *buf)
{
	size_t b = 0;

	b += pack_int((int*)a->type, buf);

	return b;
}
