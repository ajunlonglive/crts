#ifndef SHARED_SIM_ALIGNMENT_H
#define SHARED_SIM_ALIGNMENT_H

#include <stddef.h>
#include <stdint.h>

struct alignment_ele {
	uint16_t motivation;
	uint8_t motivator;
};

struct alignment {
	struct {
		size_t len;
		size_t cap;
		struct alignment_ele *e;
	} motivators;

	uint8_t max;
};

struct alignment *alignment_init(void);
uint16_t alignment_adjust(struct alignment *algn, const uint8_t id, uint16_t amnt);
void alignment_inspect(struct alignment *a);
#endif
