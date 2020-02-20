#ifndef __ALIGNMENT_H
#define __ALIGNMENT_H

#include <stddef.h>
#include <stdint.h>

struct alignment_ele {
	uint8_t motivator;
	uint16_t motivation;
};

struct alignment {
	uint8_t max;

	struct {
		size_t len;
		size_t cap;
		struct alignment_ele *e;
	} motivators;
};

struct alignment *alignment_init(void);
uint16_t alignment_adjust(struct alignment *algn, const uint8_t id, uint16_t amnt);
void alignment_inspect(struct alignment *a);
#endif
