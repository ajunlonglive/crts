#include "posix.h"

#include <string.h>

#include "shared/serialize/geom.h"
#include "shared/types/geom.h"

size_t
unpack_circle(struct circle *p, const char *buf)
{
	memcpy(p, buf, sizeof(struct circle));

	return sizeof(struct circle);
}

size_t
pack_circle(const struct circle *p, char *buf)
{
	memcpy(buf, p, sizeof(struct circle));

	return sizeof(struct circle);
}

size_t
unpack_point(struct point *p, const char *buf)
{
	memcpy(p, buf, sizeof(struct point));

	return sizeof(struct point);
}

size_t
pack_point(const struct point *p, char *buf)
{
	memcpy(buf, p, sizeof(struct point));

	return sizeof(struct point);
}

size_t
unpack_rectangle(struct rectangle *r, const char *buf)
{
	memcpy(r, buf, sizeof(struct rectangle));

	return sizeof(struct rectangle);
}

size_t
pack_rectangle(const struct rectangle *r, char *buf)
{
	memcpy(buf, r, sizeof(struct rectangle));

	return sizeof(struct rectangle);
}
