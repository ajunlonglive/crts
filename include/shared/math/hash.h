#ifndef SHARED_MATH_HASH_H
#define SHARED_MATH_HASH_H

#include <stdint.h>

uint64_t fnv_1a_64(uint32_t size, const uint8_t *key);
uint32_t fnv_1a_32(uint32_t size, const uint8_t *key);
uint64_t murmur_64(uint32_t size, const uint8_t *key);
uint32_t murmur_32(uint32_t size, const uint8_t *key);
#endif
