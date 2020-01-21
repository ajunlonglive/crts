#include "util/log.h"
#include "sim/alignment.h"

#define STEP 10
#define TOTAL_ALIGNMENT 1000

struct alignment *
alignment_init(void)
{
	struct alignment *algn = malloc(sizeof(struct alignment));

	algn->acap = STEP;
	algn->ele = calloc(algn->acap, sizeof(struct alignment_ele));

	algn->alen = 1;
	algn->ele[0].motivator = 0;
	algn->ele[0].motivation = TOTAL_ALIGNMENT;
	algn->max = 0;

	return algn;
};

static int
alignment_recalc_max(struct alignment *algn)
{
	size_t i;

	int maxi = 0, maxv = -1;

	for (i = 0; i < algn->alen; i++) {
		if (algn->ele[i].motivation > maxv) {
			maxv = algn->ele[i].motivation;
			maxi = algn->ele[i].motivator;
		}
	}

	return maxi;
}

static int
algn_index(const struct alignment *a, const int id)
{
	size_t i;

	for (i = 0; i < a->alen; i++) {
		if (a->ele[i].motivator == id) {
			return i;
		}
	}

	return -1;
}

static int
add_motivator(struct alignment *algn, const int id)
{
	int i = algn->alen;

	if (algn->alen++ > algn->acap) {
		algn->acap += STEP;
		algn->ele = realloc(algn->ele, algn->acap * sizeof(struct alignment_ele));
	}

	algn->ele[i].motivator = id;
	algn->ele[i].motivation = 0;

	return algn->alen - 1;
}

int
alignment_adjust(struct alignment *algn, const int id, int amnt)
{
	int i, index, rem;

	amnt %= TOTAL_ALIGNMENT;

	if ((index = algn_index(algn, id)) == -1) {
		index = add_motivator(algn, id);
	}

	if (algn->alen <= 1) {
		return amnt;
	}

	i = algn->alen - 1;
	rem = amnt / (algn->alen - 1);

	for (i = 0; (size_t)i < algn->alen; i++) {
		if (i == index) {
			continue;
		}

		algn->ele[i].motivation -= rem;
		if (algn->ele[i].motivation < 0) {
			algn->ele[i].motivation = 0;
		}
	}

	rem = amnt % (algn->alen - 1);
	algn->ele[index].motivation += amnt - rem;

	if (algn->ele[index].motivation > TOTAL_ALIGNMENT) {
		algn->ele[index].motivation = TOTAL_ALIGNMENT;
	}

	algn->max = alignment_recalc_max(algn);
	return rem;
};

void
alignment_inspect(struct alignment *a)
{
	size_t i;

	L("aligned to %d (contenders: %ld)", a->max, (long)a->alen);

	for (i = 0; i < a->alen; i++) {
		L("    motivator #%ld, %3d @ %5d", (long)i, a->ele[i].motivator, a->ele[i].motivation);
	}
}
