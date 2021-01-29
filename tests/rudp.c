#include "posix.h"

#include "shared/msgr/msgr.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/platform/sockets/dummy.h"
#include "shared/util/log.h"

void
handler(struct msgr *msgr, enum message_type mt, void *msg, struct msg_sender *sender)
{
	L("id:%d:msg:%s", sender->id, inspect_message(mt, msg));
}


int
main(int argc, const char *argv[])
{
	log_init();

	const struct sock_impl *socks = get_sock_impl(sock_impl_type_dummy);
	struct msgr m_client = { 0 }, m_server = { 0 };

	msgr_init(&m_client, NULL, handler, sock_impl_dummy_conf.client_id);
	msgr_init(&m_server, NULL, handler, sock_impl_dummy_conf.server_id);

	struct msgr_transport_rudp_ctx client_rudp_ctx = { 0 };
	msgr_transport_init_rudp(&client_rudp_ctx, &m_client, socks, NULL);
	struct msgr_transport_rudp_ctx server_rudp_ctx = { 0 };
	msgr_transport_init_rudp(&server_rudp_ctx, &m_server, socks, NULL);

	rudp_connect(&m_client, &sock_impl_dummy_conf.server);
	rudp_connect(&m_server, &sock_impl_dummy_conf.client);

	sock_impl_dummy_conf.reliability = 1.0;
	sock_impl_dummy_conf.cb = rudp_recv_cb;
	sock_impl_dummy_conf.client_ctx = &m_client;
	sock_impl_dummy_conf.server_ctx = &m_server;

	msgr_queue(&m_client, mt_req, &(struct msg_req) { 0 }, 0);

	L("send");
	msgr_send(&m_client);
}
