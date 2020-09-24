#ifndef SHARED_NET_DEFS_H
#define SHARED_NET_DEFS_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

#define BUFSIZE 4096
#define MAX_CXS 32
#define MSG_ID_LIM 512lu
#define FRAME_LEN MSG_ID_LIM
#define MSG_RESEND_AFTER 4
#define MSG_DESTROY_AFTER 2

extern socklen_t socklen;

typedef uint16_t msg_seq_t;
typedef uint32_t msg_ack_t;
typedef uint32_t cx_bits_t;

enum msg_flags {
	msgf_forget       = 1 << 0,
	msgf_drop_if_full = 1 << 1,
	msgf_must_send    = 1 << 2,
	msgf_ack          = 1 << 3,
	msg_flags_max     = msgf_forget
			    | msgf_drop_if_full
			    | msgf_must_send
			    | msgf_ack
};

struct msg_hdr {
	bool ack;
	msg_seq_t seq;
};

#endif
