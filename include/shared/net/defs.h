#ifndef SHARED_NET_DEFS_H
#define SHARED_NET_DEFS_H

#include <arpa/inet.h>
#include <stdint.h>
#include <sys/socket.h>

#define BUFSIZE 4096
#define FRAMELEN 32
#define MAX_CXS 32

extern socklen_t socklen;

typedef uint16_t msg_seq_t;
typedef uint32_t msg_ack_t;
typedef uint32_t cx_bits_t;

struct msg_hdr {
	msg_seq_t msg_seq;
	msg_seq_t ack_seq;
	msg_ack_t ack;
};

#endif
