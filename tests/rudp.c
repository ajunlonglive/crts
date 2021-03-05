#include "posix.h"
#include "shared/math/rand.h"
#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/msgr/transport/rudp/recv.h"
#include "shared/platform/sockets/dummy.h"
#include "shared/util/log.h"

static const struct sock_impl *socks;

enum sender {
	sender_server,
	sender_client,
};

struct ctx {
	enum sender sender;
	uint32_t recvd, sent;
	uint16_t sent_id, recvd_id;
	struct darr sent_list;
	struct hash recvd_list;
};

struct endpoint {
	struct msgr msgr;
	struct ctx ctx;
	struct msgr_transport_rudp_ctx rudp_ctx;
};

struct msginfo {
	uint16_t seq;
};

static void
ctx_init(struct ctx *ctx)
{
	ctx->sent_id = ctx->recvd_id = 0;
	hash_init(&ctx->recvd_list, 2048, 2);
	darr_init(&ctx->sent_list, 2);
}

static void
queue_msg(struct endpoint *og)
{
	enum message_type mt;
	void *dat;

	mt = mt_ent;
	dat = &(struct msg_ent) { .id = og->ctx.sent_id };
	darr_push(&og->ctx.sent_list, &og->ctx.sent_id);
	++og->ctx.sent_id;

	msgr_queue(&og->msgr, mt, dat, 0, priority_normal);
	++og->ctx.sent;
}

void
recv_handler(struct msgr *msgr, enum message_type mt, void *_msg,
	struct msg_sender *sender)
{
	struct ctx *ctx = msgr->usr_ctx;

	assert(mt == mt_ent);


	struct msg_ent *msg = _msg;

	hash_set(&ctx->recvd_list, &msg->id, 1);

	if (msg->id > ctx->recvd_id) {
		ctx->recvd_id = msg->id;
	} else {
		/* L("duplicate message recieved"); */
	}

	++ctx->recvd;
}

static void
endpoint_init(struct endpoint *ep, enum sender s, uint16_t id,
	struct sock_addr *other, void **dummy_ctx)
{
	ep->ctx = (struct ctx){ .sender = s };
	ctx_init(&ep->ctx);
	msgr_init(&ep->msgr, &ep->ctx, recv_handler, id);
	msgr_transport_init_rudp(&ep->rudp_ctx, &ep->msgr, socks, NULL);
	rudp_connect(&ep->msgr, other);
	*dummy_ctx = &ep->msgr;
}

int
main(int argc, const char *argv[])
{
	log_init();

	rand_set_seed(99);

	sock_impl_dummy_conf.reliability = 0.8;
	sock_impl_dummy_conf.cb = rudp_recv_cb;

	socks = get_sock_impl(sock_impl_type_dummy);

	struct endpoint client = { 0 };
	struct endpoint server = { 0 };

	endpoint_init(&client, sender_client, sock_impl_dummy_conf.client_id,
		&sock_impl_dummy_conf.server, &sock_impl_dummy_conf.client_ctx);

	endpoint_init(&server, sender_server, sock_impl_dummy_conf.server_id,
		&sock_impl_dummy_conf.client, &sock_impl_dummy_conf.server_ctx);

	uint32_t l;
	for (l = 0; l < 100; ++l) {
		L("\n--- client\n");
		/* for (i = 0; i < 8; ++i) { */
		queue_msg(&client);
		msgr_send(&client.msgr);
		/* } */
		L("\n--- server\n");

		queue_msg(&server);
		msgr_send(&server.msgr);
	}

	L("-------------- summary --------------\n");

	L("sock_impl_dummy_conf.reilability: %.f%%",
		sock_impl_dummy_conf.reliability * 100.0f);

	double recvd_ratio = (double)server.ctx.recvd_list.len
			     / (double)client.ctx.sent_list.len;

	L("sent: %ld, recvd: %ld, dropped: %0.0f%%", client.ctx.sent_list.len,
		server.ctx.recvd_list.len, (1.0 - recvd_ratio) * 100.0);

	L("client stats: ");
	rudp_print_stats(&client.msgr);

	L("server stats: ");
	rudp_print_stats(&server.msgr);
}
