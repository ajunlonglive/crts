#ifndef SERVER_NET_WRAPPED_MESSAGE_H
#define SERVER_NET_WRAPPED_MESSAGE_H
#include "shared/messaging/client_message.h"

struct wrapped_message {
	struct client_message cm;
	struct connection *cx;
};
#endif
