#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/base.h"
#include "shared/util/log.h"

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

MAKE_SERIALIZERS(bool);
MAKE_SERIALIZERS(char);
MAKE_SERIALIZERS(int);
MAKE_SERIALIZERS(long);
MAKE_SERIALIZERS(uint16_t);
MAKE_SERIALIZERS(uint32_t);
MAKE_SERIALIZERS(uint8_t);
MAKE_SERIALIZERS(size_t);

#undef MAKE_SERIALIZERS
