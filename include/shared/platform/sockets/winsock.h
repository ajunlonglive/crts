#ifndef SHARED_PLATFORM_SOCKETS_WINSOCK_H
#define SHARED_PLATFORM_SOCKETS_WINSOCK_H
#include "shared/platform/sockets/common.h"

extern const struct sock_impl sock_impl_system;

#ifndef NDEBUG
extern bool socket_reliability_set;
extern double socket_reliability;
#endif
#endif
