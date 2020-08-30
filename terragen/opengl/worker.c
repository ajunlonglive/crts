#include "posix.h"

#include <pthread.h>
#include <string.h>

#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "terragen/opengl/worker.h"

pthread_t worker_thread = { 0 };
pthread_attr_t worker_thread_attr = { 0 };

static _Atomic bool running = false;

static void *
genworld_worker(void *_ctx)
{
	struct ui_ctx *ctx = _ctx;

	running = true;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	terragen(&ctx->ctx, NULL);

	running = false;

	return NULL;
}

void
init_genworld_worker(void)
{
	pthread_attr_init(&worker_thread_attr);
}

void
cancel_genworld_worker(void)
{
	if (running) {
		L("canceling worker");
		pthread_cancel(worker_thread);
		running = false;
	}
}

void
start_genworld_worker(struct ui_ctx *ctx)
{
	cancel_genworld_worker();

	terragen_init(&ctx->ctx, ctx->opts);

	pthread_create(&worker_thread, &worker_thread_attr, genworld_worker, ctx);
}
