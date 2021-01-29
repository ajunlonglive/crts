#include "posix.h"

#include "shared/math/rand.h"
#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/platform/sockets/dummy.h"
#include "shared/util/log.h"

static uint32_t recvd = 0, sent = 0;

void
recv_handler(struct msgr *msgr, enum message_type mt, void *msg,
	struct msg_sender *sender)
{
	L("id:%d:msg:%s", sender->id, inspect_message(mt, msg));

	++recvd;
}

static void
send_random_msg(struct msgr *msgr)
{
	enum message_type mt;
	void *dat;

	switch ((mt = rand_uniform(message_type_count))) {
	case mt_poke:
		return;
	case mt_req:
		dat = &(struct msg_req) { .dat = { .chunk = { 32, 32 } } };
		break;
	case mt_ent:
		dat = &(struct msg_ent) { .id = 69 };
		break;
	case mt_action:
		dat = &(struct msg_action) { .dat = { .add = { .type = at_harvest } } };
		break;
	case mt_tile:
		dat = &(struct msg_tile) {
			.cp = { 48, 48 },
			.c = 1,
			.height = 2,
			.t = 3,
		};
		break;
	case mt_chunk:
		dat = &(struct msg_chunk) {
			.dat = {
				.cp = { 16, 16 },
				.tiles = { 1, 2, 3, 4, 5 }, /* ... */
				.heights = { 1, 2, 3, 4, 5 }, /* ... */
			}
		};
		break;
	default:
		assert(false);
		break;
	}

	msgr_queue(msgr, mt, dat, 0);
}

int
main(int argc, const char *argv[])
{
	log_init();

	const struct sock_impl *socks = get_sock_impl(sock_impl_type_dummy);
	struct msgr m_client = { 0 }, m_server = { 0 };

	msgr_init(&m_client, NULL, recv_handler, sock_impl_dummy_conf.client_id);
	msgr_init(&m_server, NULL, recv_handler, sock_impl_dummy_conf.server_id);

	struct msgr_transport_rudp_ctx client_rudp_ctx = { 0 };
	msgr_transport_init_rudp(&client_rudp_ctx, &m_client, socks, NULL);
	struct msgr_transport_rudp_ctx server_rudp_ctx = { 0 };
	msgr_transport_init_rudp(&server_rudp_ctx, &m_server, socks, NULL);

	rudp_connect(&m_client, &sock_impl_dummy_conf.server);
	rudp_connect(&m_server, &sock_impl_dummy_conf.client);

	sock_impl_dummy_conf.reliability = 0.7;
	sock_impl_dummy_conf.cb = rudp_recv_cb;
	sock_impl_dummy_conf.client_ctx = &m_client;
	sock_impl_dummy_conf.server_ctx = &m_server;

	uint32_t i;
	for (i = 0; i < 10; ++i) {
		send_random_msg(&m_client);
		++sent;
	}

	msgr_send(&m_client);
	L("sent: %d, recvd: %d", sent, recvd);
}
