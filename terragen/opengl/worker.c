#include "posix.h"

#include <pthread.h>
#include <string.h>

#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "terragen/opengl/worker.h"

pthread_t worker_thread = { 0 };
pthread_attr_t worker_thread_attr = { 0 };

static void *
genworld_worker(void *_ctx)
{
	struct ui_ctx *ctx = _ctx;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	terragen(&ctx->ctx, NULL);

	return NULL;
}

void
init_genworld_worker(void)
{
	pthread_attr_init(&worker_thread_attr);
}

void
start_genworld_worker(struct ui_ctx *ctx)
{
	static bool running = false;

	if (running) {
		pthread_cancel(worker_thread);
	}

	terragen_init(&ctx->ctx, ctx->opts);

	pthread_create(&worker_thread, &worker_thread_attr, genworld_worker, ctx);

	running = true;
}
