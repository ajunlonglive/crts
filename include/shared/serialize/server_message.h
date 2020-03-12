#ifndef SHARED_SERIALIZE_SERVER_MESSAGE_H
#define SHARED_SERIALIZE_SERVER_MESSAGE_H

#include "shared/messaging/server_message.h"

size_t pack_sm(const void *sm, char *buf);
size_t unpack_sm(struct server_message *ud, const char *buf);
#endif
