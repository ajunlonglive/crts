#include "posix.h"

#include <math.h>
#include <soundio/soundio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/geom.h"
#include "shared/math/rand.h"
#include "shared/sound/core.h"
#include "shared/util/log.h"
#include "tracy.h"

enum sound_msg_type {
	sound_msg_add,
	sound_msg_listener_pos,
};

struct sound_msg {
	enum sound_msg_type type; /* enum sound_msg_type */
	union {
		struct {
			float pitch;
			vec3 pos;
		} add;
		struct {
			vec3 pos;
		} listener_pos;
	} data;
};

struct sample {
	double l, r;
};

struct source {
	double rps;
	double amp;
	double pan;
	double seconds_offset;
	double duration;
	double dist;
	vec3 pos;
};

#define MAX_SOURCES 64

struct write_ctx {
	struct source sources[MAX_SOURCES];
	uint32_t sources_len;
	vec3 listener;
};

static void
add_source(struct write_ctx *ctx, vec3 pos, float pitch)
{
	if (ctx->sources_len < MAX_SOURCES) {
		const float jitter = 440.0;
		pitch += (drand48() * jitter) - (jitter / 2);

		ctx->sources[ctx->sources_len] = (struct source) {
			.rps = pitch * 2.0 * PI,
			.amp = 1.0,
			.duration = 1.0,
		};

		memcpy(ctx->sources[ctx->sources_len].pos, pos, sizeof(float) * 3);

		++ctx->sources_len;
	}
}

static void
prune_sources(struct write_ctx *ctx)
{

	int32_t i;
	for (i = ctx->sources_len - 1; i >= 0; --i) {
		if (ctx->sources[i].amp > 0.01) {
			continue;
		}

		--ctx->sources_len;

		if ((uint32_t)i == ctx->sources_len) {
			continue;
		}

		memcpy(&ctx->sources[i], &ctx->sources[ctx->sources_len],
			sizeof(struct source));
	}
}

static void
reposition_sources(struct write_ctx *ctx)
{
	uint32_t i;
	double d;

	const double r_min = 20.0;
	const double r_max = 450.0;
	const double r_factor = ((r_min - r_max) * (r_min - r_max));
	const double pan_width = 50.0;

	for (i = 0; i < ctx->sources_len; ++i) {
		d = sqrt(sqdist3d(ctx->listener, ctx->sources[i].pos));

		if (d > r_max) {
			ctx->sources[i].dist = 0.0;
		} else if (d < r_min) {
			ctx->sources[i].dist = 1.0;
		} else {
			d -= r_max;
			ctx->sources[i].dist = (d * d) / r_factor;
		}
		ctx->sources[i].dist  *= 0.3;

		d = ctx->listener[0] - ctx->sources[i].pos[0];

		if (d < -pan_width) {
			ctx->sources[i].pan = 0.0;
		} else if (d > pan_width) {
			ctx->sources[i].pan = 1.0;
		} else {
			ctx->sources[i].pan = (d + pan_width) / (pan_width * 2);
		}
	}
}

static void
process_messages(struct sound_ctx *sctx, struct write_ctx *wctx)
{
	struct sound_msg *msgs = (struct sound_msg *)soundio_ring_buffer_read_ptr(sctx->buf);
	uint32_t len = soundio_ring_buffer_fill_count(sctx->buf) / sizeof(struct sound_msg);
	uint32_t i;

	for (i = 0; i < len; ++i) {
		switch (msgs[i].type) {
		case sound_msg_add:
			add_source(wctx, msgs[i].data.add.pos, msgs[i].data.add.pitch);
			break;
		case sound_msg_listener_pos:
			memcpy(wctx->listener, msgs[i].data.listener_pos.pos, sizeof(vec3));
			break;
		}
	}

	soundio_ring_buffer_advance_read_ptr(sctx->buf, len * sizeof(struct sound_msg));
}

void
write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
	static struct write_ctx wctx = { 0 };
	const struct SoundIoChannelLayout *layout = &outstream->layout;
	struct SoundIoChannelArea *areas;
	struct sound_ctx *ctx = outstream->userdata;
	const double seconds_per_frame = 1.0 / ctx->outstream->sample_rate;
	uint32_t framei = 0, i;
	int32_t tmp;
	int err;
	double sample;
	double samples[2];
	uint32_t frames = frame_count_max;

	process_messages(ctx, &wctx);
	prune_sources(&wctx);
	reposition_sources(&wctx);

	while (framei < frames) {
		tmp = frames - framei;

		if ((err = soundio_outstream_begin_write(outstream, &areas, &tmp))) {
			LOG_W("stream error: %s", soundio_strerror(err));
			break;
		} else if (tmp <= 0) {
			break;
		}

		const uint32_t stop_at = framei + tmp;
		assert(layout->channel_count == 2); // TODO!

		for (; framei < stop_at; ++framei) {
			samples[0] = 0;
			samples[1] = 0;
			for (i = 0; i < wctx.sources_len; ++i) {
				if (wctx.sources[i].dist <= 0.0) {
					continue;
				}

				sample = sin((wctx.sources[i].seconds_offset + framei * seconds_per_frame) * wctx.sources[i].rps)
					 * wctx.sources[i].amp
					 * wctx.sources[i].dist;

				samples[0] += sample * wctx.sources[i].pan;
				samples[1] += sample * (1.0 - wctx.sources[i].pan);

				wctx.sources[i].amp *= 0.99995;
			}

			ctx->write_sample(areas[0].ptr, samples[0]);
			areas[0].ptr += areas[0].step;

			ctx->write_sample(areas[1].ptr, samples[1]);
			areas[1].ptr += areas[1].step;
		}

		if ((err = soundio_outstream_end_write(outstream))
		    && err != SoundIoErrorUnderflow) {
			LOG_W("stream error: %s", soundio_strerror(err));
			break;
		}


	}

	for (i = 0; i < wctx.sources_len; ++i) {
		wctx.sources[i].seconds_offset = wctx.sources[i].seconds_offset + seconds_per_frame * frames;
		// TODO the old code used fmod, to achieve the same result as
		// below.  Both produce an audible clicking when the seconds
		// offset carries over from > 0.9 to < 0.1.  For short sounds
		// it is not neccessary to truncate the value, but this
		// solution would not work for e.g. sustained notes
		/* wctx.sources[i].seconds_offset -= (float)(uint32_t)wctx.sources[i].seconds_offset; */
	}
}

#define SOUND_MSG_QUEUE_LEN 64
struct {
	struct sound_msg msgs[SOUND_MSG_QUEUE_LEN];
	uint32_t len;
} sound_msg_queue;

void
sc_trigger(struct sound_ctx *ctx, vec3 pos, float pitch)
{
	if (sound_msg_queue.len < SOUND_MSG_QUEUE_LEN) {
		sound_msg_queue.msgs[sound_msg_queue.len] = (struct sound_msg){
			.type = sound_msg_add,
			.data = { .add = { .pitch = pitch, .pos = { pos[0], pos[1], pos[2] } } }
		};

		++sound_msg_queue.len;
	}
}

void
sc_update(struct sound_ctx *ctx, vec3 listener)
{
	static vec3 old_listener;

	if (sound_msg_queue.len < SOUND_MSG_QUEUE_LEN) {
		if (memcmp(listener, old_listener, sizeof(vec3)) != 0) {
			memcpy(old_listener, listener, sizeof(vec3));

			sound_msg_queue.msgs[sound_msg_queue.len] = (struct sound_msg){
				.type = sound_msg_listener_pos,
				.data = { .listener_pos = { .pos = { listener[0], listener[1], listener[2] } } }
			};
			++sound_msg_queue.len;
		}
	}

	struct sound_msg *msgs = (struct sound_msg *)soundio_ring_buffer_write_ptr(ctx->buf);
	uint32_t buflen = soundio_ring_buffer_free_count(ctx->buf) / sizeof(struct sound_msg);

	if (!buflen) {
		return;
	}

	buflen = buflen > sound_msg_queue.len ? sound_msg_queue.len : buflen;

	memcpy(msgs, sound_msg_queue.msgs, buflen * sizeof(struct sound_msg));
	soundio_ring_buffer_advance_write_ptr(ctx->buf, buflen * sizeof(struct sound_msg));

	if (buflen < sound_msg_queue.len) {
		sound_msg_queue.len -= buflen;
		memmove(sound_msg_queue.msgs, &sound_msg_queue.msgs[buflen], sound_msg_queue.len * sizeof(struct sound_msg));
	} else {
		sound_msg_queue.len = 0;
	}
}
