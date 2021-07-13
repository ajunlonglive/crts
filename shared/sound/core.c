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
	sound_msg_add_3d,
	sound_msg_listener,
};

struct sound_msg {
	enum sound_msg_type type; /* enum sound_msg_type */
	union {
		struct {
			enum audio_asset asset;
			enum audio_flags flags;
			vec3 pos;
		} add_3d;
		struct {
			enum audio_asset asset;
			enum audio_flags flags;
		} add;
		struct {
			vec3 pos;
			float vol;
		} listener;
	} data;
};

struct sample {
	double l, r;
};

struct source {
	enum audio_asset asset;
	enum audio_flags flags;
	float bufi;
	float speed;
	double amp;
	double ampl;
	double ampr;
	vec3 pos;
	bool _3d;
};

#define MAX_SOURCES 64

struct write_ctx {
	struct source sources[MAX_SOURCES];
	uint32_t sources_len;
	float vol;
	vec3 listener;
};

static void
add_source(struct sound_ctx *ctx, struct write_ctx *wctx, vec3 pos, enum audio_asset asset, enum audio_flags flags, bool _3d)
{
	if (wctx->sources_len < MAX_SOURCES) {
		if (!ctx->assets[asset].data) {
			return;
		}

		wctx->sources[wctx->sources_len] = (struct source) { .asset = asset, .flags = flags };

		if (flags & audio_flag_rand) {
			wctx->sources[wctx->sources_len].speed = 0.5f + drand48();
			wctx->sources[wctx->sources_len].amp = drand48() * 0.25 + 0.5;
		} else {
			wctx->sources[wctx->sources_len].speed = 1.0f;
			wctx->sources[wctx->sources_len].amp = 1.0;
		}

		if ((wctx->sources[wctx->sources_len]._3d = _3d)) {
			memcpy(wctx->sources[wctx->sources_len].pos, pos, sizeof(float) * 3);
		}

		++wctx->sources_len;
	}
}

static void
prune_sources(struct sound_ctx *ctx, struct write_ctx *wctx)
{
	uint32_t samplei;
	int32_t i;
	for (i = wctx->sources_len - 1; i >= 0; --i) {
		if (wctx->sources[i].flags & audio_flag_loop) {
			continue;
		}

		samplei = wctx->sources[i].bufi * 2;
		if (samplei < ctx->assets[wctx->sources[i].asset].len) {
			continue;
		}

		--wctx->sources_len;

		if ((uint32_t)i == wctx->sources_len) {
			continue;
		}

		memcpy(&wctx->sources[i], &wctx->sources[wctx->sources_len],
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
	double pan, dist;

	for (i = 0; i < ctx->sources_len; ++i) {
		if (!ctx->sources[i]._3d) {
			ctx->sources[i].ampl = 1.0f;
			ctx->sources[i].ampr = 1.0f;
			continue;
		}

		d = sqrt(sqdist3d(ctx->listener, ctx->sources[i].pos));

		if (d > r_max) {
			dist = 0.0;
		} else if (d < r_min) {
			dist = 1.0;
		} else {
			d -= r_max;
			dist = (d * d) / r_factor;
		}
		dist  *= 0.3;

		d = ctx->listener[0] - ctx->sources[i].pos[0];

		if (d < -pan_width) {
			pan = 0.0;
		} else if (d > pan_width) {
			pan = 1.0;
		} else {
			pan = (d + pan_width) / (pan_width * 2);
		}

		ctx->sources[i].ampl = ctx->sources[i].amp * dist * pan;
		ctx->sources[i].ampr = ctx->sources[i].amp * dist * (1.0 - pan);
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
			add_source(sctx, wctx, (vec3) { 0, 0, 0 }, msgs[i].data.add.asset, msgs[i].data.add.flags, false);
			break;
		case sound_msg_add_3d:
			add_source(sctx, wctx, msgs[i].data.add_3d.pos, msgs[i].data.add_3d.asset, msgs[i].data.add_3d.flags, true);
			break;
		case sound_msg_listener:
			memcpy(wctx->listener, msgs[i].data.listener.pos, sizeof(vec3));
			wctx->vol = msgs[i].data.listener.vol * 10;
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
	assert(ctx->outstream->sample_rate == 48000); // TODO!
	uint32_t framei = 0, i, samplei;
	int32_t tmp;
	int err;
	double samples[2], sl, sr;
	uint32_t frames = frame_count_max;
	float sample_blend;

	if (frames > 2048) {
		frames = 2048;
	}

	process_messages(ctx, &wctx);
	prune_sources(ctx, &wctx);
	reposition_sources(&wctx);

	while (framei < frames) {
		tmp = frames - framei;

		if ((err = soundio_outstream_begin_write(outstream, &areas, &tmp))) {
			LOG_W(log_sound, "stream error: %s", soundio_strerror(err));
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
				samplei = wctx.sources[i].bufi;
				sample_blend = wctx.sources[i].bufi - (float)samplei;
				samplei *= 2;

				if (samplei >= ctx->assets[wctx.sources[i].asset].len) {
					if (wctx.sources[i].flags & audio_flag_loop) {
						/* we really should blend the last sample with
						 * the first sample, but I'm not implementing
						 * it yet because I don't know if I'll end up
						 * needing pitch-shifted + looping sounds
						 */
						wctx.sources[i].bufi = 0.0f;
						samplei = 0;
						sample_blend = 0.0f;
					} else {
						continue;
					}
				}

				assert(samplei + 3 < ctx->assets[wctx.sources[i].asset].len + 2);

				sl = (ctx->assets[wctx.sources[i].asset].data[samplei] * sample_blend) +
				     ctx->assets[wctx.sources[i].asset].data[samplei + 2] * (1.0f - sample_blend);

				sr = (ctx->assets[wctx.sources[i].asset].data[samplei + 1] * sample_blend) +
				     ctx->assets[wctx.sources[i].asset].data[samplei + 3] * (1.0f - sample_blend);

				/* L(log_sound, "bufi: %f, blend: %f, samplei: %d (%d) | %f, %f", wctx.sources[i].bufi, */
				/* 	sample_blend, samplei, samplei + 1, sl, sr); */


				samples[0] += sl * wctx.sources[i].ampl;
				samples[1] += sr * wctx.sources[i].ampr;

				wctx.sources[i].bufi += wctx.sources[i].speed;
			}

			ctx->write_sample(areas[0].ptr, samples[0] * wctx.vol);
			areas[0].ptr += areas[0].step;

			ctx->write_sample(areas[1].ptr, samples[1] * wctx.vol);
			areas[1].ptr += areas[1].step;
		}

		if ((err = soundio_outstream_end_write(outstream))
		    && err != SoundIoErrorUnderflow) {
			LOG_W(log_sound, "stream error: %s", soundio_strerror(err));
			break;
		}
	}
}

#define SOUND_MSG_QUEUE_LEN 64
struct {
	struct sound_msg msgs[SOUND_MSG_QUEUE_LEN];
	uint32_t len;
} sound_msg_queue;

void
sc_trigger(struct sound_ctx *ctx, enum audio_asset asset, enum audio_flags flags)
{
	if (sound_msg_queue.len < SOUND_MSG_QUEUE_LEN - 1) {
		sound_msg_queue.msgs[sound_msg_queue.len] = (struct sound_msg){
			.type = sound_msg_add,
			.data.add = {
				.asset = asset,
				.flags = flags,
			}
		};

		++sound_msg_queue.len;
	}
}

void
sc_trigger_3d(struct sound_ctx *ctx, vec3 pos, enum audio_asset asset, enum audio_flags flags)
{

	/* check against SOUND_MSG_QUEUE_LEN - 1 so we always have room for the
	 * listener message */
	if (sound_msg_queue.len < SOUND_MSG_QUEUE_LEN - 1) {
		sound_msg_queue.msgs[sound_msg_queue.len] = (struct sound_msg){
			.type = sound_msg_add_3d,
			.data.add_3d = {
				.asset = asset,
				.pos = { pos[0], pos[1], pos[2] },
				.flags = flags,
			}
		};

		++sound_msg_queue.len;
	}
}

void
sc_update(struct sound_ctx *ctx, vec3 listener)
{

	assert(sound_msg_queue.len < SOUND_MSG_QUEUE_LEN);
	sound_msg_queue.msgs[sound_msg_queue.len] = (struct sound_msg){
		.type = sound_msg_listener,
		.data.listener = {
			.pos = { listener[0], listener[1], listener[2] },
			.vol = ctx->vol,
		}
	};
	++sound_msg_queue.len;

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
