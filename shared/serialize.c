#include <string.h>
#include "world.h"
#include "log.h"
#include "serialize.h"

/*
   static void log_bytes(const char *bytes, size_t n)
   {
        size_t i;

        for (i = 0; i < n; i++)
                printf("%08x ", bytes[i]);

        printf("\n");
   }
 */

static size_t unpack_int(int *i, const char *buf)
{
	memcpy(i, buf, sizeof(int));
	//log_bytes((char*)i, sizeof(int));
	//L("unpacked int %d (%d bytes)", *i, sizeof(int));

	return sizeof(int);
}

static size_t pack_int(const int *i, char *buf)
{
	//L("packing int %d (%d bytes)", *i, sizeof(int));
	memcpy(buf, i, sizeof(int));
	//log_bytes(buf, sizeof(int));

	return sizeof(int);
}

static size_t unpack_char(char *i, const char *buf)
{
	memcpy(i, buf, sizeof(char));

	return sizeof(char);
}

static size_t pack_char(const char *i, char *buf)
{
	memcpy(buf, &i, sizeof(char));

	return sizeof(char);
}

static size_t unpack_point(struct point *p, const char *buf)
{
	memcpy(p, buf, sizeof(struct point));

	return sizeof(struct point);
}

static size_t pack_point(const struct point *p, char *buf)
{
	memcpy(buf, p, sizeof(struct point));

	return sizeof(struct point);
}

size_t pack_ent(struct ent *e, char *buf)
{
	size_t b = 0;

	b += pack_point(&e->pos, buf);
	b += pack_char(&e->c, &buf[b]);

	return b;
}

size_t unpack_ent(struct ent *e, const char *buf)
{
	size_t b = 0;

	b += unpack_point(&e->pos, buf);
	b += unpack_char(&e->c, &buf[b]);

	return b;
}

size_t unpack_action(struct action *a, const char *buf)
{
	size_t b = 0;

	b += unpack_int((int*)&a->type, buf);
	a->motivator = -1;
	a->id = -1;

	return b;
}

size_t pack_action(struct action *a, char *buf)
{
	size_t b = 0;

	b += pack_int((int*)a->type, buf);

	return b;
}

size_t unpack_ent_update(struct ent_update *eu, const char *buf)
{
	size_t b = 0;

	b += unpack_int(&eu->id, buf);
	b += unpack_point(&eu->pos, &buf[b]);

	return b;
}

size_t pack_ent_update(const struct ent_update *eu, char *buf)
{
	size_t b = 0;

	b += pack_int(&eu->id, buf);
	b += pack_point(&eu->pos, &buf[b]);

	return b;
}

size_t unpack_update(struct update *ud, const char *buf)
{
	return unpack_int((int*)&ud->type, buf);
}

size_t pack_update(const struct update *ud, char *buf)
{
	return pack_int((int*)&ud->type, buf);
}
