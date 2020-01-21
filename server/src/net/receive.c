#define _POSIX_C_SOURCE 201900L

#include <string.h>
#include <time.h>

#include "pool.h"
#include "receive.h"
#include "serialize/client_message.h"
#include "sim/action.h"
#include "types/queue.h"
#include "util/log.h"
#include "wrapped_message.h"

#define BUFSIZE 255
#define HEAP_SIZE 255

struct message_heap {
	struct {
		struct wrapped_message e[HEAP_SIZE];
		size_t i;
	} wrapper;

	struct {
		struct cm_action e[HEAP_SIZE];
		size_t i;
	} action;

	struct {
		struct cm_chunk_req e[HEAP_SIZE];
		size_t i;
	} chunk_req;

};

static long start_sec;
static struct message_heap *mh;

static void
init_timer(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	start_sec = ts.tv_sec;
}

static long
elapsed_ms(void)
{
	long ms, ooms;
	static long oms = 0;
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);

	ooms = oms;
	oms = ms = (((ts.tv_sec - start_sec) * 1000) + (ts.tv_nsec / 1000000));
	return ms - ooms;
}

static void
wrap_inc(size_t *i)
{
	*i = *i >= HEAP_SIZE - 1 ? 0 : *i + 1;
}

static struct wrapped_message *
unpack_message(struct message_heap *mh, const char *buf)
{
	size_t b;
	struct wrapped_message *wm;

	wm = &mh->wrapper.e[mh->wrapper.i];
	wrap_inc(&mh->wrapper.i);

	b = unpack_cm(&wm->cm, buf);

	switch (wm->cm.type) {
	case client_message_poke:
		break;
	case client_message_action:
		b += unpack_cm_action(&mh->action.e[mh->action.i], &buf[b]);
		wm->cm.update = &mh->action.e[mh->action.i];

		wrap_inc(&mh->action.i);
		break;
	case client_message_chunk_req:
		b += unpack_cm_chunk_req(&mh->chunk_req.e[mh->chunk_req.i], &buf[b]);
		wm->cm.update = &mh->chunk_req.e[mh->chunk_req.i];

		wrap_inc(&mh->chunk_req.i);
		break;
	}

	return wm;
}

void
net_receive_init(void)
{
	mh = malloc(sizeof(struct message_heap));
	memset(mh, 0, sizeof(struct message_heap));
	init_timer();
}

void
net_receive(struct server *s)
{
	char buf[BUFSIZE];
	int b;
	const struct connection *cx;
	struct wrapped_message *wm;

	union {
		struct sockaddr_in ia;
		struct sockaddr sa;
	} caddr;

	cx_prune(s->cxs, elapsed_ms());

	while ((b = recvfrom(s->sock, buf, BUFSIZE, 0, &caddr.sa, &socklen)) >= 1) {
		cx = cx_establish(s->cxs, &caddr.ia);

		wm = unpack_message(mh, buf);
		wm->cx = cx;

		queue_push(s->inbound, wm);
	}
}
