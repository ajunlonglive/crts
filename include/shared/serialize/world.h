#ifndef SHARED_SERIALIZE_WORLD_H
#define SHARED_SERIALIZE_WORLD_H

#include "shared/sim/chunk.h"

size_t pack_chunk(const struct chunk *c, uint8_t *buf, uint32_t len);
size_t unpack_chunk(struct chunk *c, const uint8_t *buf, uint32_t len);
#endif
