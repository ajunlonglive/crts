#ifndef SHARED_PLATFORM_POSIX_THREAD_H
#define SHARED_PLATFORM_POSIX_THREAD_H

#include <pthread.h>

struct thread {
	pthread_t thread;
};
#endif
