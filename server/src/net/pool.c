#include <arpa/inet.h>
#include <string.h>

#include "util/log.h"
#include "pool.h"

#define STEP 5
//ms before disconnect client
#define STALE_THRESHOLD 1000


struct cx_pool *cx_pool_init()
{
	struct cx_pool *cp = malloc(sizeof(struct cx_pool));

	memset(cp, 0, sizeof(struct cx_pool));

	cp->cxs = hash_init(sizeof(struct sockaddr_in));

	return cp;
}

static struct connection *cx_add(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection *cl;

	cp->mem.len++;

	if (cp->mem.len > cp->mem.cap) {
		cp->mem.cap += STEP;
		cp->mem.cxs = realloc(cp->mem.cxs, sizeof(struct connection) * cp->mem.cap);
		memset(&cp->mem.cxs[cp->mem.cap - STEP], 0, sizeof(struct connection) * STEP);
		L("increased cx capacity to %d elemenets", cp->mem.cap);
	}

	cl = &cp->mem.cxs[cp->mem.len - 1];
	hash_set(cp->cxs, addr, cl);
	cx_init(cl, addr);

	cl->saddr = *addr;

	// TODO set this so that each client is unique
	cl->motivator = 1; //s->cxs.next_motivator++;
	L("new client connected!");
	cx_inspect(cl);

	return cl;
}

struct connection *cx_establish(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection *cl;

	if ((cl = hash_get(cp->cxs, addr)) == NULL )
		cl = cx_add(cp, addr);

	return cl;
}

static void remove_client(struct cx_pool *cp, size_t id)
{
	struct connection *cl;

	L("lost client[%d] %d", id, cp->mem.cxs[id].motivator);

	cp->mem.len--;

	if (cp->mem.len == 0) {
		memset(cp->mem.cxs, 0, sizeof(struct connection));
		return;
	}

	cl = &cp->mem.cxs[cp->mem.len];
	memmove(&cp->mem.cxs[id], cl, sizeof(struct connection));
	memset(cl, 0, sizeof(struct connection));
}

void cx_prune(struct cx_pool *cp, long ms)
{
	size_t i;
	struct connection *cl;

	for (i = 0; i < cp->mem.len; i++) {
		cl = &cp->mem.cxs[i];
		cl->stale += ms;

		if (cl->stale >= STALE_THRESHOLD)
			remove_client(cp, i);
	}
}

