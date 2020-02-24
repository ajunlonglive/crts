#include <stddef.h>

#include "client/hiface.h"
#include "shared/messaging/client_message.h"
#include "shared/sim/chunk.h"
#include "shared/sim/world.h"
#include "shared/types/hash.h"
#include "shared/types/queue.h"
#include "shared/util/log.h"

#define REQUEST_COOLDOWN 30

struct reqd_chunks {
	struct hash *h;
	int *e;
	size_t len;
	size_t cap;
};

static struct hash *rq;

void
request_missing_chunks_init(void)
{
	rq = hash_init(2048, 8, sizeof(struct point));
}

void
request_missing_chunks(struct hiface *hif, const struct rectangle *r)
{
	unsigned nv;
	const uint16_t *val;
	struct point onp, np = onp = nearest_chunk(&hif->view);

	for (; np.x < hif->view.x + r->width; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < hif->view.y + r->height; np.y += CHUNK_SIZE) {
			if (hash_get(hif->sim->w->chunks->h, &np) == NULL) {
				if ((val = hash_get(rq, &np)) == NULL || *val > REQUEST_COOLDOWN) {
					L("requesting chunk @ %d, %d", np.x, np.y);
					queue_push(hif->sim->outbound, cm_create(client_message_chunk_req, &np));

					nv = 0;
				} else {
					nv = *val + 1;
				}

				hash_set(rq, &np, nv);
			}
		}
	}
}
