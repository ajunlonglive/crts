#ifndef SHARED_SERIALIZE_MISC_H
#define SHARED_SERIALIZE_MISC_H

#include "shared/net/msg_queue.h"
#include "shared/net/ack.h"

size_t pack_msg_hdr(const struct msg_hdr *ag, char *buf);
size_t unpack_msg_hdr(struct msg_hdr *ag, const char *buf);

size_t pack_acks(const struct acks *a, char *buf);
size_t unpack_acks(struct acks *a, const char *buf);
#endif
