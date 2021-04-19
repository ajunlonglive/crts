#ifndef SHARED_UTIL_TIMER_H
#define SHARED_UTIL_TIMER_H

#include "posix.h"

#include <stdint.h>
#include <time.h>

struct timer_avg {
	float avg, val;
};

struct timer {
	struct timespec start;
};

void timer_init(struct timer *pt);
float timer_lap(struct timer *pt);
float timer_read(const struct timer *t);
void timer_avg_push(struct timer_avg *timer_avg, float val);
#endif
