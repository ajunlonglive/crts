#include "posix.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "shared/platform/common/thread.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

struct thread_wrapper_ctx {
	thread_func func;
	void *usr_ctx;
};

static DWORD
thread_wrapper(void *_ctx)
{
	struct thread_wrapper_ctx *ctx = _ctx;

	ctx->func(ctx->usr_ctx);

	z_free(ctx);

	return 0;
}

void
thread_cancel(struct thread *thread)
{
	TerminateThread(thread->handle, 0);
}

bool
thread_create(struct thread *thread, thread_func func, void *usr_ctx)
{
	struct thread_wrapper_ctx *ctx = z_calloc(1, sizeof(struct thread_wrapper_ctx));
	ctx->usr_ctx = usr_ctx;
	ctx->func = func;

	thread->handle = CreateThread(
		NULL,           // default security attributes
		0,              // use default stack size
		thread_wrapper, // thread function name
		ctx,            // argument to thread function
		0,              // use default creation flags
		&thread->id);   // returns the thread identifier

	return true;
}
