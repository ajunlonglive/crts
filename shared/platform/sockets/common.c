#include "posix.h"

#include <assert.h>
#include <stdio.h>

#include "shared/platform/sockets/common.h"
#include "shared/platform/sockets/dummy.h"

#if defined(CRTS_PLATFORM_POSIX)
#include "shared/platform/sockets/berkeley.h"
#else
#error "no valid platform defined"
#endif

const struct sock_impl *
get_sock_impl(enum sock_impl_type type)
{
	switch (type) {
	case sock_impl_type_system:
		return &sock_impl_system;
	case sock_impl_type_dummy:
		return &sock_impl_dummy;
	default:
		assert(false);
		return 0;
	}
}

static uint16_t
bswap_16(uint16_t x)
{
	return x << 8 | x >> 8;
}

static uint16_t
ntohs(uint16_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_16(n) : n;
}

const char *
sock_addr_to_s(const struct sock_addr *addr)
{
	static char buf[32] = { 0 };
	unsigned char *a = (void *)&addr->addr;
	snprintf(buf, 32, "%d.%d.%d.%d:%d", a[0], a[1], a[2], a[3],
		ntohs(addr->port));
	return buf;
}
