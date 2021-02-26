#ifndef SHARED_MSGR_MSGR_H
#define SHARED_MSGR_MSGR_H

#include "shared/serialize/message.h"

typedef uint32_t msg_addr_t;

struct msgr;
struct msg_sender;

typedef void (*msgr_transport_send)(struct msgr *msgr);
typedef void (*msgr_transport_recv)(struct msgr *msgr);
typedef void (*msgr_transport_queue)(struct msgr *msgr, struct message *msg,
	msg_addr_t dest);
typedef void (*msg_handler)(struct msgr *msgr, enum message_type mt, void *msg,
	struct msg_sender *sender);

struct msg_sender {
	msg_addr_t addr;
	uint16_t id;
};

enum msgr_transport_impl {
	msgr_transport_basic,
	msgr_transport_rudp,
};

struct msgr {
	struct {
		struct message msg;
		msg_addr_t dest;
		bool full;
	} msg_buf;

	msgr_transport_send send;
	msgr_transport_recv recv;
	msgr_transport_queue queue;
	void *transport_ctx;
	msg_handler handler;
	void *usr_ctx;
	uint16_t id;

	enum msgr_transport_impl transport_impl;
};

void msgr_init(struct msgr *, void *usr_ctx, msg_handler handler, uint16_t id);
void msgr_send(struct msgr *msgr);
void msgr_recv(struct msgr *msgr);
void msgr_queue(struct msgr *msgr, enum message_type mt, void *msg,
	msg_addr_t dest);
#endif
