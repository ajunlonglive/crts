#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/base.h"
#include "shared/util/log.h"

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

#define MAKE_SERIALIZERS(type) \
	size_t \
	unpack_ ## type(type * i, const char *buf) \
	{ \
		memcpy(i, buf, sizeof(type)); \
 \
		return sizeof(type); \
	} \
 \
	size_t \
		pack_ ## type(const type * i, char *buf) \
	{ \
		memcpy(buf, i, sizeof(type)); \
 \
		return sizeof(type); \
	}

MAKE_SERIALIZERS(uint8_t);
MAKE_SERIALIZERS(uint16_t);
MAKE_SERIALIZERS(uint32_t);
MAKE_SERIALIZERS(long);
MAKE_SERIALIZERS(int);
MAKE_SERIALIZERS(char);

#undef MAKE_SERIALIZERS
