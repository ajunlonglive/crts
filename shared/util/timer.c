#include "posix.h"

#include "shared/util/timer.h"

#define FACTOR 0.1f

void
timer_avg_push(struct timer_avg *avg, float val)
{
	avg->val = val;
	avg->avg = val * FACTOR + (avg->avg * (1.0f - FACTOR));
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
