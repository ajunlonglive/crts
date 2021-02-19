#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "shared/types/sack.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

enum sack_flags {
	sackf_deleted = 1 << 0,
};

struct sack_hdr {
	uint16_t ilen;
	uint8_t flags;
};

void
sack_init(struct sack *sk, uint8_t hdr_len, uint32_t cap,
	sack_stuffing_func stuff)
{
	*sk = (struct sack){
		.cap = cap,
		.hdr_len = hdr_len,
		.mem = z_calloc(cap, 1),
		.stuff = stuff,
	};
}

void
sack_destroy(struct sack *sk)
{
	z_free(sk);
}

void
sack_stuff(struct sack *sk, void *hdr, void *itm)
{
	uint16_t total_hdr_len = sizeof(struct sack_hdr) + sk->hdr_len;

	assert(sk->len + total_hdr_len < sk->cap);

	struct sack_hdr *skhdr = (struct sack_hdr *)&sk->mem[sk->len];
	sk->len += sizeof(struct sack_hdr);

	memcpy(&sk->mem[sk->len], hdr, sk->hdr_len);
	sk->len += sk->hdr_len;

	skhdr->ilen = sk->stuff(itm, &sk->mem[sk->len], sk->cap - sk->len);
	assert(skhdr->ilen);
	sk->len += skhdr->ilen;

	++sk->items;
}

static void
sack_compact(struct sack *sk)
{
	if (!sk->len) {
		return;
	}

	uint32_t i, newlen = 0;
	uint16_t payload_size;
	uint8_t *slot = NULL;
	struct sack_hdr *skhdr;

	for (i = 0; i < sk->len;) {
		skhdr = (struct sack_hdr *)&sk->mem[i];
		payload_size = sizeof(struct sack_hdr) + sk->hdr_len + skhdr->ilen;

		if (!(skhdr->flags & sackf_deleted)) {
			if (slot) {
				/* L("%d@%d moving -> %ld", payload_size, i, slot - sk->mem); */
				memmove(slot, &sk->mem[i], payload_size);
				slot += payload_size;
				/* } else { */
				/* 	L("%d@%d keeping", payload_size, i); */
			}
			newlen += payload_size;
		} else {
			if (!slot) {
				/* L("%d@%d marking slot", payload_size, i); */
				slot = &sk->mem[i];
			}
		}

		i += payload_size;
	}

	memset(&sk->mem[newlen], 0, sk->len - newlen);
	sk->len = newlen;
}

void
sack_iter(struct sack *sk, void *ctx, sack_iter_cb cb)
{
	struct sack_hdr *skhdr;
	void *hdr, *itm;
	uint32_t i;
	bool deleted = false;

	for (i = 0; i < sk->len;) {
		skhdr = (struct sack_hdr *)&sk->mem[i];
		i += sizeof(struct sack_hdr);
		hdr = &sk->mem[i];
		i += sk->hdr_len;
		itm = &sk->mem[i];
		i += skhdr->ilen;

		switch (cb(ctx, hdr, itm, skhdr->ilen)) {
		case dir_cont:
			continue;
		case dir_break:
			goto finished;
		case dir_del:
			skhdr->flags |= sackf_deleted;
			--sk->items;
			deleted = true;
			break;
		}
	}

finished:
	if (deleted) {
		sack_compact(sk);
	}
}
