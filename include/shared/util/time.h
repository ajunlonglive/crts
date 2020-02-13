#ifndef __UTIL_TIME_H
#define __UTIL_TIME_H

#define NS_IN_S 1000000000

#include <time.h>

long sleep_remaining(struct timespec *start, long dur, long slept_ns);
#endif
