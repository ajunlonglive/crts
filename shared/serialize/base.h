#ifndef __SERIALIZE_BASE_H
#define __SERIALIZE_BASE_H
#include <stdlib.h>
void log_bytes(const char *bytes, size_t n);
size_t unpack_int(int *i, const char *buf);
size_t pack_int(const int *i, char *buf);
size_t unpack_long(long *i, const char *buf);
size_t pack_long(const long *i, char *buf);
size_t unpack_char(char *i, const char *buf);
size_t pack_char(const char *i, char *buf);
#endif
