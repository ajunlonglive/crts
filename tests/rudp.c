#include "posix.h"
#include "shared/math/rand.h"
#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/msgr/transport/rudp/recv.h"
#include "shared/platform/sockets/dummy.h"
#include "shared/util/log.h"

enum sender {
	sender_server,
	sender_client,
};

struct ctx {
	enum sender sender;
	uint32_t recvd, sent;
	int32_t sent_id, recvd_id;
	struct darr sent_list;
	struct hash recvd_list;
};

struct msginfo {
	uint16_t seq;
};

static void
queue_msg(struct ctx *ctx, struct msgr *msgr)
{
	enum message_type mt;
	void *dat;

	mt = mt_ent;
	dat = &(struct msg_ent) { .id = ctx->sent_id };
	darr_push(&ctx->sent_list, &ctx->sent_id);
	++ctx->sent_id;

	msgr_queue(msgr, mt, dat, 0);
	++ctx->sent;
}

static void
ctx_init(struct ctx *ctx)
{
	ctx->sent_id = ctx->recvd_id = 0;
	hash_init(&ctx->recvd_list, 2048, 2);
	darr_init(&ctx->sent_list, 2);
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

int
main(int argc, const char *argv[])
{
	log_init();

	rand_set_seed(99);

	sock_impl_dummy_conf.reliability = 0.80;
	sock_impl_dummy_conf.cb = rudp_recv_cb;

	const struct sock_impl *socks = get_sock_impl(sock_impl_type_dummy);

	struct msgr m_client = { 0 };
	struct ctx client_ctx = { .sender = sender_client };
	struct msgr_transport_rudp_ctx client_rudp_ctx = { 0 };
	ctx_init(&client_ctx);
	msgr_init(&m_client, &client_ctx, recv_handler, sock_impl_dummy_conf.client_id);
	msgr_transport_init_rudp(&client_rudp_ctx, &m_client, socks, NULL);
	rudp_connect(&m_client, &sock_impl_dummy_conf.server);
	sock_impl_dummy_conf.client_ctx = &m_client;

	struct msgr m_server = { 0 };
	struct ctx server_ctx = { .sender = sender_server };
	struct msgr_transport_rudp_ctx server_rudp_ctx = { 0 };
	ctx_init(&server_ctx);
	msgr_init(&m_server, &server_ctx, recv_handler, sock_impl_dummy_conf.server_id);
	msgr_transport_init_rudp(&server_rudp_ctx, &m_server, socks, NULL);
	rudp_connect(&m_server, &sock_impl_dummy_conf.client);
	sock_impl_dummy_conf.server_ctx = &m_server;

	uint32_t l;
	for (l = 0; l < UINT16_MAX; ++l) {
		queue_msg(&client_ctx, &m_client);
		msgr_send(&m_client);

		msgr_send(&m_server);
	}

	L("reilability: %.f%%", sock_impl_dummy_conf.reliability * 100.0f);

	L("sent: %ld, recvd: %ld, %0.0f", client_ctx.sent_list.len, server_ctx.recvd_list.len,
		(double)server_ctx.recvd_list.len / (double)client_ctx.sent_list.len * 100.0);

	rudp_print_stats(&m_client);
	rudp_print_stats(&m_server);
}
