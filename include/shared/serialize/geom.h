#ifndef __SERIALIZE_GEOM_H
#define __SERIALIZE_GEOM_H
#include <stdlib.h>
#include "shared/types/geom.h"

size_t unpack_circle(struct circle *p, const char *buf);
size_t pack_circle(const struct circle *p, char *buf);
size_t unpack_point(struct point *p, const char *buf);
size_t pack_point(const struct point *p, char *buf);
size_t unpack_rectangle(struct rectangle *r, const char *buf);
size_t pack_rectangle(const struct rectangle *r, char *buf);
#endif
