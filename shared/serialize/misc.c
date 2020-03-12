#include <stdio.h>
#include <assert.h>

#include "shared/serialize/base.h"
#include "shared/serialize/misc.h"
#include "shared/util/log.h"

size_t
pack_msg_hdr(const struct msg_hdr *ag, char *buf)
{
	size_t b = 0;

	b += pack_uint16_t(&ag->msg_seq, &buf[b]);
	b += pack_uint16_t(&ag->ack_seq, &buf[b]);
	b += pack_uint32_t(&ag->ack, &buf[b]);

	assert(b == MSG_HDR_LEN);

	return b;
}

size_t
unpack_msg_hdr(struct msg_hdr *ag, const char *buf)
{
	size_t b = 0;

	b += unpack_uint16_t(&ag->msg_seq, &buf[b]);
	b += unpack_uint16_t(&ag->ack_seq, &buf[b]);
	b += unpack_uint32_t(&ag->ack, &buf[b]);

	assert(b == MSG_HDR_LEN);

	return b;
}
