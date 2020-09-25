#ifndef SHARED_SERIALIZE_MISC_H
#define SHARED_SERIALIZE_MISC_H

#include "shared/net/msg_queue.h"

size_t pack_msg_hdr(const struct msg_hdr *mh, uint8_t *buf, uint32_t blen);
size_t unpack_msg_hdr(struct msg_hdr *mh, const uint8_t *buf, uint32_t blen);

size_t pack_acks(struct hash *a, uint8_t *buf, uint32_t blen);
size_t unpack_acks(struct hash *a, const uint8_t *buf, uint32_t blen);

size_t pack_hello(const struct msg_hello *msg, uint8_t *buf, uint32_t blen);
size_t unpack_hello(struct msg_hello *msg, const uint8_t *buf, uint32_t blen);
#endif
