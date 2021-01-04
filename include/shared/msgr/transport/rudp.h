#ifndef SHARED_MSGR_TRANSPORT_RUDP_H
#define SHARED_MSGR_TRANSPORT_RUDP_H
#include "shared/msgr/msgr.h"
#include "shared/platform/sockets/common.h"

void msgr_transport_connect(struct msgr *msgr, struct sock_addr *addr);
bool msgr_transport_init_rudp(struct msgr *msgr, const struct sock_impl *impl,
	struct sock_addr *bind_addr);
#endif
