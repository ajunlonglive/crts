#include <stddef.h>

#include "client/hiface.h"
#include "client/net.h"
#include "client/request_missing_chunks.h"
#include "shared/messaging/client_message.h"
#include "shared/sim/chunk.h"
#include "shared/sim/world.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

#define REQUEST_COOLDOWN 30

struct reqd_chunks {
	struct hash *h;
	size_t *e;
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
request_missing_chunks(struct hiface *hif, const struct rectangle *r,
	struct net_ctx *nx)
{
	unsigned nv;
	const size_t *val;
	struct point onp, np = onp = nearest_chunk(&hif->view);

	for (; np.x < hif->view.x + r->width; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < hif->view.y + r->height; np.y += CHUNK_SIZE) {
			if (hdarr_get(hif->sim->w->chunks->hd, &np) == NULL) {
				if ((val = hash_get(rq, &np)) == NULL || *val > REQUEST_COOLDOWN) {
					L("requesting chunk %d, %d", np.x, np.y);
					send_msg(nx, client_message_chunk_req, &np);

					nv = 0;
				} else {
					nv = *val + 1;
				}

				hash_set(rq, &np, nv);
			}
		}
	}
}
