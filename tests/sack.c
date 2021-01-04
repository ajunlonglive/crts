#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/math/rand.h"
#include "shared/types/sack.h"
#include "shared/util/log.h"

enum item_type {
	it_a = 77,
	it_b = 32,
	it_c = 11,
};

struct item_a {
	enum item_type t;
	uint8_t pad[10];
};
struct item_a item_a = { .t = it_a };

struct item_b {
	enum item_type t;
	const char *c;
	uint8_t pad[40];
};
struct item_b item_b = { .t = it_b };

struct item_c {
	enum item_type t;
	uint8_t pad[99];
};
struct item_c item_c = { .t = it_c };

struct hdr {
	enum item_type type;
};

static size_t
packing_func(void *item, uint8_t *buf, uint32_t blen)
{
	enum item_type *t = item;
	uint32_t len;

	switch (*t) {
	case it_a:
		len = sizeof(struct item_a);
		break;
	case it_b:
		len = sizeof(struct item_b);
		break;
	case it_c:
		len = sizeof(struct item_c);
		break;
	default:
		assert(false);
	}

	assert(blen > len);
	memcpy(buf, item, len);
	return len;
}

static void *
random_struct(enum item_type *t)
{
	double r = drand48();

	if (r < 0.33) {
		*t = it_a;
		return &item_a;
	} else if (r < 0.66) {
		*t = it_b;
		return &item_b;
	} else {
		*t = it_c;
		return &item_c;
	}
}

static uint32_t deleted = 0;

static enum del_iter_result
iter_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct hdr *hdr = _hdr;
	switch (hdr->type) {
	case it_a:
		assert(sizeof(struct item_a) == len);
		assert(((struct item_a *)itm)->t == it_a);
		break;
	case it_b:
		assert(sizeof(struct item_b) == len);
		assert(((struct item_b *)itm)->t == it_b);
		break;
	case it_c:
		assert(sizeof(struct item_c) == len);
		assert(((struct item_c *)itm)->t == it_c);
		break;
	default:
		assert(false);
	}

	if (drand48() > 0.5) {
		/* L("o it@%p, %d", (void *)itm, *(enum item_type *)itm); */

		return dir_cont;
	} else {
		/* L("x it@%p, %d", (void *)itm, *(enum item_type *)itm); */
		++deleted;
		return dir_del;
	}
}

int
main(void)
{
	log_init();
	log_level = ll_debug;

	struct sack sk = { 0 };
	sack_init(&sk, sizeof(struct hdr), 1024 * 32, packing_func);

	rand_set_seed(3);

	uint32_t i, j;
	struct hdr hdr = { 0 };
	void *itm;

	uint32_t len = 0;

	for (i = 0; i < 1024; ++i) {
		for (j = 0; j < 128; ++j) {
			++len;
			itm = random_struct(&hdr.type);
			/* L("stuffing %d", hdr.type); */
			sack_stuff(&sk, &hdr, itm);
		}

		sack_iter(&sk, NULL, iter_cb);
		deleted = 0;
	}
}
