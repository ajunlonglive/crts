#include "posix.h"

#include <stddef.h>

#include "client/hiface.h"
#include "client/net.h"
#include "client/request_missing_chunks.h"
#include "shared/sim/chunk.h"
#include "shared/sim/world.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

#define REQUEST_COOLDOWN 3
#define REQUEST_EXTRA (CHUNK_SIZE * 1)

static struct hash rq = { 0 };

void
request_missing_chunks_init(void)
{
	hash_init(&rq, 2048, sizeof(struct point));
}

static void
request_chunk(struct point *np, struct net_ctx *nx)
{
	size_t nv;
	const size_t *val;

	if ((val = hash_get(&rq, np)) == NULL || *val > REQUEST_COOLDOWN) {
		struct msg_req msg = {
			.mt = rmt_chunk,
			.dat = { .chunk = *np }
		};

		queue_msg(nx, mt_req, &msg, nx->cxs.cx_bits, msgf_forget);

		nv = 0;
	} else {
		nv = *val + 1;
	}

	hash_set(&rq, np, nv);
}

void
request_missing_chunks(struct hiface *hif, const struct rectangle *r,
	struct net_ctx *nx)
{
	struct rectangle l = {
		.pos = {
			hif->view.x - REQUEST_EXTRA,
			hif->view.y - REQUEST_EXTRA
		},
		.width = r->width + REQUEST_EXTRA * 2,
		.height = r->height + REQUEST_EXTRA * 2,
	};

	struct point onp, np = onp = nearest_chunk(&l.pos);

	for (; np.x < l.pos.x + l.width; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < l.pos.y + l.height; np.y += CHUNK_SIZE) {
			if (hdarr_get(&hif->sim->w->chunks.hd, &np) != NULL) {
				continue;
			}

			if (np.x > 0 && np.y > 0) {
				request_chunk(&np, nx);
			}
		}
	}
}
