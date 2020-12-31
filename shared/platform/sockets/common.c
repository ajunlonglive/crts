#include "posix.h"

#include <assert.h>

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
