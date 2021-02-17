#include "posix.h"

#include "shared/serialize/byte_swappers.h"

static uint16_t
bswap_16(uint16_t x)
{
	return x << 8 | x >> 8;
}

static uint32_t
bswap_32(uint32_t x)
{
	return x >> 24 | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) | x << 24;
}

uint16_t
host_to_net_16(uint16_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_16(n) : n;
}

uint16_t
net_to_host_16(uint16_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_16(n) : n;
}

uint32_t
host_to_net_32(uint32_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_32(n) : n;
}

uint32_t
net_to_host_32(uint32_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_32(n) : n;
}
