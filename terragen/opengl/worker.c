#include "posix.h"

#include <string.h>

#include "shared/platform/common/thread.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "terragen/opengl/worker.h"

struct thread worker_thread;

static _Atomic bool running = false;

static void
genworld_worker(void *_ctx)
{
	struct ui_ctx *ctx = _ctx;

	running = true;

	terragen(&ctx->ctx, NULL);

	running = false;
}

void
init_genworld_worker(void)
{
}

void
cancel_genworld_worker(void)
{
	if (running) {
		L(log_misc, "canceling worker");
		thread_cancel(&worker_thread);
		running = false;
	}
}

void
start_genworld_worker(struct ui_ctx *ctx)
{
	cancel_genworld_worker();

	terragen_init(&ctx->ctx, ctx->opts);

	thread_create(&worker_thread, genworld_worker, ctx);
}
