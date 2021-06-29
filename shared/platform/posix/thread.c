#include "posix.h"

#include <string.h>
#include <pthread.h>

#include "shared/platform/common/thread.h"
#include "shared/util/log.h"

typedef void *((*thread_func)(void *ctx));

bool
thread_create(struct thread *thread, thread_func func, void *ctx)
{
	int err = 0;
	pthread_attr_t attr;
	if ((err = pthread_attr_init(&attr)) != 0) {
		goto err;
	}

	if ((err = pthread_create(&thread->thread, &attr, func, ctx)) != 0) {
		goto err;
	}

	return true;
err:
	LOG_W(log_misc, "failed to create thread: %s", strerror(err));
	return false;
}
