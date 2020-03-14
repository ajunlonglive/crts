#include <stdio.h>
#include <assert.h>

#include "shared/serialize/base.h"
#include "shared/serialize/net.h"
#include "shared/util/log.h"

size_t
pack_msg_hdr(const struct msg_hdr *ag, char *buf)
{
	size_t b = 0;

	b += pack_uint16_t(&ag->seq, &buf[b]);

	assert(b == MSG_HDR_LEN);

	return b;
}

size_t
unpack_msg_hdr(struct msg_hdr *ag, const char *buf)
{
	size_t b = 0;

	b += unpack_uint16_t(&ag->seq, &buf[b]);

	assert(b == MSG_HDR_LEN);

	return b;
}

size_t
pack_acks(const struct acks *a, char *buf)
{
	size_t b = 0, i;

	b += pack_uint16_t(&a->leader, &buf[b]);

	for (i = 0; i < ACK_BLOCKS; ++i) {
		b += pack_uint32_t(&a->acks[i], &buf[b]);
	}

	return b;
}

size_t
unpack_acks(struct acks *a, const char *buf)
{
	size_t b = 0, i;

	b += unpack_uint16_t(&a->leader, &buf[b]);

	for (i = 0; i < ACK_BLOCKS; ++i) {
		b += unpack_uint32_t(&a->acks[i], &buf[b]);
	}

	a->initialized = true;


	return b;
}
