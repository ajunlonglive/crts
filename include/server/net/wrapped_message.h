#ifndef __WRAPPED_MESSAGE_H
#define __WRAPPED_MESSAGE_H
#include "shared/messaging/client_message.h"

struct wrapped_message {
	struct client_message cm;
	const struct connection *cx;
};
#endif
