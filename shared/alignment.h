#ifndef __ALIGNMENT_H
#define __ALIGNMENT_H
#include <stdlib.h>

struct alignment_ele {
	int motivator;
	int motivation;
};

struct alignment {
	size_t alen;
	size_t acap;

	int max;

	struct alignment_ele *ele;
};

struct alignment *alignment_init(void);
int alignment_adjust(struct alignment *algn, int motivator, int amnt);
#endif
