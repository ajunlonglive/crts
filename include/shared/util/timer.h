#ifndef SHARED_UTIL_TIMER_H
#define SHARED_UTIL_TIMER_H

#include "posix.h"

#include <stdint.h>
#include <time.h>

#define TIMER_SMA_LEN 32

struct timer_sma {
	float buf[TIMER_SMA_LEN], sum, avg;
	uint32_t bufi;
};

struct timer {
	struct timespec start;
};

void timer_init(struct timer *pt);
float timer_lap(struct timer *pt);
float timer_read(const struct timer *t);
void timer_sma_push(struct timer_sma *timer_sma, float val);
#endif
