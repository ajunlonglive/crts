#ifndef __SERIALIZE_BASE_H
#define __SERIALIZE_BASE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAKE_SERIALIZERS(type) \
	size_t unpack_ ## type(type * i, const char *buf); \
	size_t pack_ ## type(const type * i, char *buf);

MAKE_SERIALIZERS(bool);
MAKE_SERIALIZERS(char);
MAKE_SERIALIZERS(int);
MAKE_SERIALIZERS(long);
MAKE_SERIALIZERS(uint16_t);
MAKE_SERIALIZERS(uint32_t);
MAKE_SERIALIZERS(uint8_t);

#undef MAKE_SERIALIZERS

#define unpack_enum(enum_type, struct, buffer, b) do { \
		memcpy(struct, buffer, sizeof(enum enum_type)); \
		b += sizeof(enum enum_type); \
} while (0);
#define pack_enum(enum_type, struct, buffer, b) do { \
		memcpy(buffer, struct, sizeof(enum enum_type)); \
		b += sizeof(enum enum_type); \
} while (0);
#endif
