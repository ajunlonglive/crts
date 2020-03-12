#ifndef SHARED_SERIALIZE_MISC_H
#define SHARED_SERIALIZE_MISC_H

#include "shared/net/msg_queue.h"

#define MSG_HDR_LEN sizeof(struct msg_hdr)

size_t pack_msg_hdr(const struct msg_hdr *ag, char *buf);
size_t unpack_msg_hdr(struct msg_hdr *ag, const char *buf);
#endif
