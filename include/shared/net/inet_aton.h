#ifndef SHARED_NET_INET_ATON_H
#define SHARED_NET_INET_ATON_H
#include <netinet/in.h>

int inet_aton(const char *s0, struct in_addr *dest);
#endif
