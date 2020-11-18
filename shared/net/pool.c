#include "posix.h"

#include <arpa/inet.h>
#include <string.h>

#include "shared/net/connection.h"
#include "shared/net/pool.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

//ms before remove connection
#define STALE_THRESHOLD 1000

static const void *
connection_key_getter(void *_cx)
{
	struct connection *cx = _cx;

	return &cx->addr;
}

void
cx_pool_init(struct cx_pool *cp)
{
	hdarr_init(&cp->cxs, 16, sizeof(struct sockaddr_in),
		sizeof(struct connection), connection_key_getter);
}

static msg_ack_t
get_available_bit(struct cx_pool *cp)
{
	size_t i;
	msg_ack_t b;

	for (i = 0; i < MAX_CXS; ++i) {
		b = 1 << i;
		if (!(cp->cx_bits & b)) {
			cp->cx_bits |= b;
			return b;
		}
	}

	return MAX_CXS;
}

struct connection *
cx_add(struct cx_pool *cp, struct sockaddr_in *addr, uint16_t id)
{
	struct connection cl;
	cx_bits_t bit;

	if ((bit = get_available_bit(cp)) == MAX_CXS) {
		L("cxs full, rejecting new connection");
		return NULL;
	}

	cx_init(&cl, addr);
	cl.id = id;
	cl.bit = bit;

	cl.new = true;

	L("got new connection");
	cx_inspect(&cl);

	/* TODO: now that we associate an id with a connection on establish,
	 * we could save space by using the id as the key rather than the
	 * address */
	hdarr_set(&cp->cxs, &cl.addr, &cl);
	return hdarr_get(&cp->cxs, &cl.addr);
}

struct connection *
cx_establish(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection *cl;

	if (!(cl = hdarr_get(&cp->cxs, addr))) {
		return NULL;
	}

	cl->stale = 0;

	return cl;
}

static void
remove_connection(struct cx_pool *cp, struct connection *cx)
{
	L("lost connection");
	cx_inspect(cx);
	cx_destroy(cx);

	cp->cx_bits &= ~cx->bit;

	hdarr_del(&cp->cxs, &cx->addr);
}

struct check_prune_ctx {
	long ms;
	struct connection *prune_me;
};

static enum iteration_result
check_prune(void *_ctx, void *_cx)
{
	struct connection *cx = _cx;
	struct check_prune_ctx *ctx = _ctx;

	cx->stale += ctx->ms;

	if (cx->stale >= STALE_THRESHOLD) {
		ctx->prune_me = cx;
	}

	return ir_cont;
}

void
cx_prune(struct cx_pool *cp, long ms)
{
	struct check_prune_ctx ctx = {
		.ms = ms,
		.prune_me = NULL,
	};

	hdarr_for_each(&cp->cxs, &ctx, check_prune);

	if (ctx.prune_me != NULL) {
		remove_connection(cp, ctx.prune_me);
	}
}

void
cx_pool_clear(struct cx_pool *cp)
{
	hdarr_clear(&cp->cxs);
	cp->cx_bits = 0;
}
