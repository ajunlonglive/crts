#include <arpa/inet.h>
#include <string.h>

#include "server/net/pool.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

//ms before disconnect client
#define STALE_THRESHOLD 1000

static void *
connection_key_getter(void *_cx)
{
	struct connection *cx = _cx;

	return &cx->addr;
}

struct cx_pool *
cx_pool_init(void)
{
	struct cx_pool *cp = malloc(sizeof(struct cx_pool));

	memset(cp, 0, sizeof(struct cx_pool));

	cp->cxs = hdarr_init(16, sizeof(struct sockaddr_in),
		sizeof(struct connection), connection_key_getter);

	return cp;
}

static struct connection *
cx_add(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection cl;

	cx_init(&cl, addr);

	hdarr_set(cp->cxs, &cl.addr, &cl);

	L("new client connected!");
	cx_inspect(&cl);

	return hdarr_get(cp->cxs, &cl.addr);
}

struct connection *
cx_establish(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection *cl;

	if ((cl = hdarr_get(cp->cxs, addr)) == NULL) {
		cl = cx_add(cp, addr);
	}

	cl->stale = 0;

	return cl;
}

static void
remove_connection(struct cx_pool *cp, struct connection *cx)
{
	L("lost client");
	cx_inspect(cx);

	hdarr_del(cp->cxs, &cx->addr);
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

	hdarr_for_each(cp->cxs, &ctx, check_prune);

	if (ctx.prune_me != NULL) {
		remove_connection(cp, ctx.prune_me);
	}
}
