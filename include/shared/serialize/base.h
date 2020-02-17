#ifndef __SERIALIZE_BASE_H
#define __SERIALIZE_BASE_H
#include <stdint.h>
#include <stddef.h>

void log_bytes(const char *bytes, size_t n);

#define MAKE_SERIALIZERS(type) \
	size_t unpack_ ## type(type * i, const char *buf); \
	size_t pack_ ## type(const type * i, char *buf);


MAKE_SERIALIZERS(uint8_t);
MAKE_SERIALIZERS(uint16_t);
MAKE_SERIALIZERS(uint32_t);
MAKE_SERIALIZERS(long);
MAKE_SERIALIZERS(int);
MAKE_SERIALIZERS(char);

#undef MAKE_SERIALIZERS
#endif
