#include "posix.h"

#include "server/aggregate_msgs.h"
#include "server/handle_msg.h"
#include "server/server.h"
#include "server/sim/ai.h"
#include "shared/pathfind/preprocess.h"
#include "shared/platform/common/thread.h"
#include "shared/util/log.h"
#include "tracy.h"

static const float sim_fps = 1.0f / 30.0f,
		   sim_sleep_fps = 250.0f;

static struct thread server_thread;
static struct ring_buffer server_ctl;

enum server_ctl_msg_type {
	server_ctl_msg_shutdown,
};

struct server_ctl_msg {
	enum server_ctl_msg_type type;
};

bool
init_server(struct server *s, struct world_loader *wl)
{
	*s = (struct server) { 0 };

	LOG_I(log_misc, "initializing server");

	ring_buffer_init(&server_ctl, sizeof(struct server_ctl_msg), 64);

	world_init(&s->w);

	if (!world_load(&s->w, wl)) {
		return false;
	}

	/* ag_init_components(&s->w.chunks); */

	sim_init(&s->w, &s->sim);

	msgr_init(&s->msgr, &s->sim, server_handle_msg, 0);

	LOG_I(log_misc, "server initialized");

	return true;
}

bool
reset_server(struct server *s, struct world_loader *wl)
{
	world_reset(&s->w);

	if (!world_load(&s->w, wl)) {
		return false;
	}

	sim_reset(&s->sim);
	ai_reset();

	return true;
}

static void
server_tick(struct server *s, uint32_t ticks)
{
	TracyCZoneAutoS;

	msgr_recv(&s->msgr);

	if (!s->sim.paused) {
		ai_tick(&s->sim);

		uint32_t i;
		for (i = 0; i < ticks; ++i) {
			simulate(&s->sim);
			// TODO performance: we should only need to do this once after
			// N ticks, but right now the simulate does cleanup that will
			// remove events before they are caught, e.g. emt_kill
			aggregate_msgs(&s->sim, &s->msgr);
		}
	}

	msgr_queue(&s->msgr, mt_server_info, &(struct msg_server_info) {
		.fps = 1.0f / s->prof.server_tick.avg,
	}, 0, priority_dont_resend);

	msgr_send(&s->msgr);

	TracyCZoneAutoE;
}

static void
server_loop(void *_ctx)
{
	struct server *s = _ctx;
	struct timer timer;
	struct timespec tick = { .tv_nsec = (1.0f / sim_sleep_fps) * 1000000000 };
	timer_init(&timer);

	float simtime = 0;
	float server_tick_time = 0;

	const uint32_t cap = 1;
	bool capped;

	struct server_ctl_msg *ctl_msg;

	while (true) {
		if ((ctl_msg = ring_buffer_pop(&server_ctl))) {
			switch (ctl_msg->type) {
			case server_ctl_msg_shutdown:
				return;
			}
		}

		uint32_t ticks = simtime / sim_fps;
		capped = false;

		if (ticks) {
			simtime -= sim_fps * ticks;
			if (ticks > cap) {
				LOG_W(log_misc, "capping server ticks @ %d (wanted %d)", cap, ticks);
				ticks = cap;
				capped = true;
			}
			server_tick(s, ticks);
		}

		server_tick_time = timer_lap(&timer);
		simtime += server_tick_time;

		if (!capped) {
			nanosleep(&tick, NULL);
		}

		timer_avg_push(&s->prof.server_tick, server_tick_time);
	}
}

void
server_start(struct server *s)
{
	if (!thread_create(&server_thread, server_loop, s)) {
		return;
	}
}

void
server_stop(void)
{
	ring_buffer_push(&server_ctl, &(struct server_ctl_msg){ .type = server_ctl_msg_shutdown });
	thread_join(&server_thread);
}
