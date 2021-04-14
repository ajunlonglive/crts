#include "posix.h"

#include "shared/util/timer.h"

void
timer_sma_push(struct timer_sma *sma, float val)
{
	uint32_t new_bufi = (sma->bufi + 1) & 31;

	sma->sum += val - sma->buf[new_bufi];

	sma->buf[new_bufi] = val;
	sma->bufi = new_bufi;

	sma->avg = sma->sum / TIMER_SMA_LEN;
}

static float
diff_time(const struct timespec *start, const struct timespec *end)
{
	float secs = end->tv_sec - start->tv_sec;
	float ns = (end->tv_nsec - start->tv_nsec) / 1000000000.0;

	/* L("%ld.%09ld - %ld.%09ld = %f", */
	/* 	end->tv_sec, end->tv_nsec, */
	/* 	start->tv_sec, start->tv_nsec, */
	/* 	secs + ns); */

	return secs + ns;
}

void
timer_init(struct timer *t)
{
	clock_gettime(CLOCK_MONOTONIC, &t->start);
}

float
timer_lap(struct timer *t)
{
	struct timespec stop;
	clock_gettime(CLOCK_MONOTONIC, &stop);

	float diff = diff_time(&t->start, &stop);

	t->start = stop;
	return diff;
}

float
timer_read(const struct timer *t)
{
	struct timespec stop;
	clock_gettime(CLOCK_MONOTONIC, &stop);

	return diff_time(&t->start, &stop);
}
