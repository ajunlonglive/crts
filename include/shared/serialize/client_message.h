#ifndef __SERIALIZE_CLIENT_MESSAGE_H
#define __SERIALIZE_CLIENT_MESSAGE_H

#include "shared/messaging/client_message.h"

size_t pack_cm(const struct client_message *ud, char *buf);
size_t unpack_cm(struct client_message *ud, const char *buf);

size_t pack_cm_action(const struct cm_action *eu, char *buf);
size_t unpack_cm_action(struct cm_action *eu, const char *buf);

size_t pack_cm_chunk_req(const struct cm_chunk_req *eu, char *buf);
size_t unpack_cm_chunk_req(struct cm_chunk_req *eu, const char *buf);
#endif
