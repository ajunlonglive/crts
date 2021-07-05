#include "posix.h"

#include <string.h>
#include <pthread.h>

#include "shared/platform/common/thread.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

struct thread_wrapper_ctx {
	thread_func func;
	void *usr_ctx;
};

static void *
thread_wrapper(void *_ctx)
{
	struct thread_wrapper_ctx *ctx = _ctx;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	ctx->func(ctx->usr_ctx);

	z_free(ctx);

	return NULL;
}

void
thread_cancel(struct thread *thread)
{
	pthread_cancel(thread->thread);
}

bool
thread_create(struct thread *thread, thread_func func, void *usr_ctx)
{
	int err = 0;
	pthread_attr_t attr;
	if ((err = pthread_attr_init(&attr)) != 0) {
		goto err;
	}

	struct thread_wrapper_ctx *ctx = z_calloc(1, sizeof(struct thread_wrapper_ctx));
	ctx->usr_ctx = usr_ctx;
	ctx->func = func;

	if ((err = pthread_create(&thread->thread, &attr, thread_wrapper, ctx)) != 0) {
		goto err;
	}

	return true;
err:
	LOG_W(log_misc, "failed to create thread: %s", strerror(err));
	return false;
}

bool
thread_join(struct thread *thread)
{
	void *dummy;
	int err;
	if ((err = pthread_join(thread->thread, &dummy)) != 0) {
		LOG_W(log_misc, "failed to create thread: %s", strerror(err));
		return false;
	}

	return true;
}
