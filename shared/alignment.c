#include "log.h"
#include "alignment.h"
#define STEP 10
#define TOTAL_ALIGNMENT 1000

struct alignment *alignment_init(void)
{
	struct alignment *algn = malloc(sizeof(struct alignment));

	algn->acap = STEP;
	algn->ele = calloc(algn->acap, sizeof(struct alignment_ele));

	algn->alen = 1;
	algn->ele[0].motivator = 0;
	algn->ele[0].motivation = TOTAL_ALIGNMENT;

	return algn;
};

static int alignment_recalc_max(struct alignment *algn)
{
	size_t i;

	int maxi = 0, maxv = -1;

	for (i = 0; i < algn->alen; i++) {
		if (algn->ele[i].motivation > maxv) {
			maxv = algn->ele[i].motivation;
			maxi = algn->ele[i].motivation;
		}
	}

	return maxi;
}

static int algn_index(const struct alignment *const a, const int id)
{
	size_t i;

	for (i = 0; i < a->alen; i++)
		if (a->ele[i].motivator == id)
			return i;

	return -1;
}

int alignment_adjust(struct alignment *algn, const int id, const int amnt)
{
	int i, index, rem;

	if (algn->alen <= 1) {
		algn->ele[0].motivation = TOTAL_ALIGNMENT;
		return amnt;
	} else if ((index = algn_index(algn, id) == -1)) {
		return amnt;
	}

	i = algn->alen - 1;
	rem = amnt / (algn->alen - 1);

	for (i = 0; (size_t)i < algn->alen; i++) {
		if (i == index)
			continue;

		algn->ele[i].motivation -= rem;
	}

	rem = amnt % (algn->alen - 1);
	algn->ele[index].motivation += amnt - rem;

	algn->max = alignment_recalc_max(algn);
	return rem;
};
