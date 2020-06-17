#include "posix.h"

/* TODO: currently unused, needs a refactor */
#include <stdbool.h>
#include <stdlib.h>

#include "shared/sim/alignment.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define TOTAL_ALIGNMENT 1000

static int
add_motivator(struct alignment *algn, const int id)
{
	union {
		void **vp;
		struct alignment_ele **mp;
	} mp = { .mp = &algn->motivators.e };
	size_t i = algn->motivators.len;

	get_mem(mp.vp, sizeof(struct alignment_ele), &algn->motivators.len,
		&algn->motivators.cap);

	algn->motivators.e[i].motivator = id;
	algn->motivators.e[i].motivation = 0;

	return algn->motivators.len - 1;
}

struct alignment *
alignment_init(void)
{
	struct alignment *algn = calloc(1, sizeof(struct alignment));
	algn->max = 0;

	add_motivator(algn, 0);
	algn->motivators.e[0].motivation = TOTAL_ALIGNMENT;

	return algn;
}

static int
recalc_max(struct alignment *algn)
{
	size_t i;

	int maxi = 0, maxv = -1;

	for (i = 0; i < algn->motivators.len; i++) {
		if (algn->motivators.e[i].motivation > maxv) {
			maxv = algn->motivators.e[i].motivation;
			maxi = algn->motivators.e[i].motivator;
		}
	}

	return maxi;
}

static bool
algn_index(const struct alignment *a, const uint8_t id, size_t *i)
{
	for (*i = 0; *i < a->motivators.len; (*i)++) {
		if (a->motivators.e[*i].motivator == id) {
			return true;
		}
	}

	return false;
}

uint16_t
alignment_adjust(struct alignment *algn, const uint8_t id, uint16_t amnt)
{
	int rem;
	size_t i, index;

	amnt %= TOTAL_ALIGNMENT;

	if (!algn_index(algn, id, &index)) {
		index = add_motivator(algn, id);
	}

	if (algn->motivators.len <= 1) {
		return amnt;
	}

	i = algn->motivators.len - 1;
	rem = amnt / (algn->motivators.len - 1);

	for (i = 0; i < algn->motivators.len; i++) {
		if (i == index) {
			continue;
		}

		algn->motivators.e[i].motivation -= rem;
	}

	rem = amnt % (algn->motivators.len - 1);
	algn->motivators.e[index].motivation += amnt - rem;

	if (algn->motivators.e[index].motivation > TOTAL_ALIGNMENT) {
		algn->motivators.e[index].motivation = TOTAL_ALIGNMENT;
	}

	algn->max = recalc_max(algn);

	return rem;
}

void
alignment_inspect(struct alignment *a)
{
	size_t i;

	L("aligned to %d (contenders: %ld)", a->max, a->motivators.len);

	for (i = 0; i < a->motivators.len; i++) {
		L("    motivator %3d @ %5d / %5d",
			a->motivators.e[i].motivator,
			a->motivators.e[i].motivation,
			TOTAL_ALIGNMENT);
	}
}
