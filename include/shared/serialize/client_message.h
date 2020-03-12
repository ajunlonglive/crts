#ifndef SHARED_SERIALIZE_CLIENT_MESSAGE_H
#define SHARED_SERIALIZE_CLIENT_MESSAGE_H

#include "shared/messaging/client_message.h"

size_t pack_cm(const void *cm, char *buf);
size_t unpack_cm(struct client_message *ud, const char *buf);
#endif
