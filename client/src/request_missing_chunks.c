#include <stddef.h>
#include "types/hash.h"
#include "hiface.h"
#include "util/log.h"
#include "sim/chunk.h"
#include "sim/world.h"
#include "types/queue.h"
#include "messaging/client_message.h"

#define REQUEST_COOLDOWN 30

struct reqd_chunks {
	struct hash *h;
	int *e;
	size_t len;
	size_t cap;
};

static struct hash *rq;

void request_missing_chunks_init(void)
{
	rq = hash_init(2048, 8, sizeof(struct point));
}

void request_missing_chunks(struct hiface *hif, const struct rectangle *r)
{
	unsigned nv;
	const struct hash_elem *he;
	struct point onp, np = onp = nearest_chunk(&hif->view);

	for (; np.x < hif->view.x + r->width; np.x += CHUNK_SIZE)
		for (np.y = onp.y; np.y < hif->view.y + r->height; np.y += CHUNK_SIZE)
			if ((he = hash_get(hif->sim->w->chunks->h, &np)) == NULL || !(he->init & HASH_VALUE_SET)) {
				he = hash_get(rq, &np);

				if (he == NULL || !(he->init & HASH_VALUE_SET) || he->val > REQUEST_COOLDOWN) {
					L("requesting chunk @ %d, %d", np.x, np.y);
					queue_push(hif->sim->outbound, cm_create(client_message_chunk_req, &np));

					nv = 0;
				} else {
					nv = he->val + 1;
				}

				hash_set(rq, &np, nv);
			}
}
