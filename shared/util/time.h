#ifndef __UTIL_TIME_H
#define __UTIL_TIME_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <time.h>

#define NS_IN_S 1000000000

long sleep_remaining(struct timespec *start, long dur, long slept_ns);
#endif
