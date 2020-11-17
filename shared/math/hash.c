#include "posix.h"

#include <string.h>

#include "shared/math/hash.h"

uint64_t
fnv_1a_64(uint32_t size, const uint8_t *key)
{
	uint64_t h = 14695981039346656037u;
	uint16_t i;

	for (i = 0; i < size; i++) {
		h ^= key[i];
		h *= 1099511628211u;
	}

	return h;
}

uint32_t
fnv_1a_32(uint32_t size, const uint8_t *key)
{
	uint32_t h = 2166136261u;
	uint16_t i;

	for (i = 0; i < size; i++) {
		h ^= key[i];
		h *= 16777619u;
	}

	return h;
}

uint64_t
murmur_64(uint32_t size, const uint8_t *key)
{
	const uint64_t seed = 0,
		       m = 0xc6a4a7935bd1e995;
	const uint32_t rest = size & 7,
		       offset = size - rest;
	uint64_t h1 = seed ^ (size * m), k1;
	uint32_t i;

	for (i = 0; i < size >> 3; ++i) {
		k1 = ((uint32_t *)key)[i];
		k1 *= m;
		k1 ^= k1 >> 47;
		k1 *= m;
		h1 ^= k1;
		h1 *= m;
	}

	if (rest > 0) {
		k1 = 0;
		memcpy(&k1, &key[offset], rest);
		h1 ^= k1;
		h1 %= m;
	}

	h1 ^= h1 >> 47;
	h1 %= m;
	h1 ^= h1 >> 47;

	return h1;
}

uint32_t
murmur_32(uint32_t size, const uint8_t *key)
{
	const uint32_t seed = 0xc70f6907,
		       m = 0x5bd1e995,
		       offset = size & 0xfffffffc,
		       rest = size & 3;

	uint32_t h1 = seed ^ size, k1, i;

	for (i = 0; i < size >> 2; ++i) {
		k1 = ((uint32_t *)key)[i];
		k1 *= m;
		k1 ^= k1 >> 24;
		k1 *= m;
		h1 *= m;
		h1 ^= k1;
	}

	if (rest >= 3) {
		h1 ^= key[offset + 2] << 16;
	}
	if (rest >= 2) {
		h1 ^= key[offset + 1] << 8;
	}
	if (rest >= 1) {
		h1 ^= key[offset + 0];
		h1 *= m;
	}

	h1 ^= h1 >> 13;
	h1 *= m;
	h1 ^= h1 >> 15;

	return h1;
}
