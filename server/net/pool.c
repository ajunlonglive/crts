#include <arpa/inet.h>
#include <string.h>

#include "server/net/pool.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define STEP 5
//ms before disconnect client
#define STALE_THRESHOLD 10000


struct cx_pool *
cx_pool_init(void)
{
	struct cx_pool *cp = malloc(sizeof(struct cx_pool));

	memset(cp, 0, sizeof(struct cx_pool));

	cp->cxs = hash_init(16, 1, sizeof(struct sockaddr_in));

	return cp;
}

static struct connection *
cx_add(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection *cl;

	union {
		void **vp;
		struct connection **cp;
	} cxp = { .cp = &cp->mem.cxs };

	int i = get_mem(cxp.vp, sizeof(struct connection), &cp->mem.len, &cp->mem.cap);

	hash_set(cp->cxs, addr, i);

	cl = cp->mem.cxs + i;

	cx_init(cl, addr);

	cl->saddr.ia = *addr;

	// TODO set this so that each client is unique
	cl->motivator = 1; //s->cxs.next_motivator++;

	L("new client connected!");
	cx_inspect(cl);

	return cl;
}

const struct connection *
cx_establish(struct cx_pool *cp, struct sockaddr_in *addr)
{
	struct connection *cl;
	const uint16_t *val;

	if ((val = hash_get(cp->cxs, addr)) != NULL) {
		cl = cp->mem.cxs + *val;
	} else {
		cl = cx_add(cp, addr);
	}

	cl->stale = 0;

	return cl;
}

static void
remove_client(struct cx_pool *cp, size_t id)
{
	struct connection *cl;

	L("lost client[%ld] %d", (long)id, cp->mem.cxs[id].motivator);

	cp->mem.len--;
	hash_unset(cp->cxs, &(cp->mem.cxs + id)->addr);

	if (cp->mem.len == 0) {
		memset(cp->mem.cxs, 0, sizeof(struct connection));
		return;
	}

	cl = &cp->mem.cxs[cp->mem.len];
	memmove(&cp->mem.cxs[id], cl, sizeof(struct connection));
	memset(cl, 0, sizeof(struct connection));
}

void
cx_prune(struct cx_pool *cp, long ms)
{
	size_t i;
	struct connection *cl;

	for (i = 0; i < cp->mem.len; i++) {
		cl = &cp->mem.cxs[i];
		cl->stale += ms;

		if (cl->stale >= STALE_THRESHOLD) {
			remove_client(cp, i);
		}
	}
}

