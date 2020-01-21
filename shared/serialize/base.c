#include <stdlib.h>
#include <string.h>
#include "util/log.h"
#include "serialize/base.h"

void
log_bytes(const char *bytes, size_t n)
{
	char buf[255];
	size_t i, j = 0;

	for (i = 0; i < n; i++) {
		j += sprintf(&buf[j], "%08x ", bytes[i]);
	}

	L("%ld bytes: %s", (long)n, buf);
}

size_t
unpack_int(int *i, const char *buf)
{
	memcpy(i, buf, sizeof(int));

	return sizeof(int);
}

size_t
pack_int(const int *i, char *buf)
{
	memcpy(buf, i, sizeof(int));

	return sizeof(int);
}

size_t
unpack_long(long *i, const char *buf)
{
	memcpy(i, buf, sizeof(long));

	return sizeof(long);
}

size_t
pack_long(const long *i, char *buf)
{
	memcpy(buf, i, sizeof(long));

	return sizeof(long);
}

size_t
unpack_char(char *i, const char *buf)
{
	memcpy(i, buf, sizeof(char));

	return sizeof(char);
}

size_t
pack_char(const char *i, char *buf)
{
	memcpy(buf, &i, sizeof(char));

	return sizeof(char);
}
