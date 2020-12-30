#ifndef SHARED_SERIALIZE_MISC_H
#define SHARED_SERIALIZE_MISC_H

#include <stddef.h>
#include <stdint.h>

typedef uint16_t msg_seq_t;

/* #include "shared/net/msg_queue.h" */

enum msg_kind {
	mk_msg,
	mk_ack,
	mk_hello,
	msg_kind_count,
};

struct msg_hdr {
	enum msg_kind kind;
	msg_seq_t seq;
};

#define VERSION_LEN 12
struct msg_hello {
	uint8_t version[VERSION_LEN];
	uint16_t id;
};

size_t pack_msg_hdr(const struct msg_hdr *mh, uint8_t *buf, uint32_t blen);
size_t unpack_msg_hdr(struct msg_hdr *mh, const uint8_t *buf, uint32_t blen);

/* size_t pack_acks(struct hash *a, uint8_t *buf, uint32_t blen); */
/* size_t unpack_acks(struct hash *a, const uint8_t *buf, uint32_t blen); */

size_t pack_hello(const struct msg_hello *msg, uint8_t *buf, uint32_t blen);
size_t unpack_hello(struct msg_hello *msg, const uint8_t *buf, uint32_t blen);
#endif
