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

struct sample {
	double l, r;
};

enum audio_asset_type {
	audio_asset_type_sfx,
	audio_asset_type_music,
};

static const enum audio_asset_type audio_asset_type[audio_asset_count] = {
	[audio_asset_theme_1] = audio_asset_type_music,
	[audio_asset_theme_2] = audio_asset_type_music,
	[audio_asset_theme_3] = audio_asset_type_music,
	[audio_asset_step_dirt] = audio_asset_type_sfx,
	[audio_asset_step_grass] = audio_asset_type_sfx,
	[audio_asset_step_rock] = audio_asset_type_sfx,
	[audio_asset_step_sand] = audio_asset_type_sfx,
	[audio_asset_die] = audio_asset_type_sfx,
	[audio_asset_spawn] = audio_asset_type_sfx,
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
	struct { float master, music, sfx; } vol;
	struct { vec3 pos; float angle; } listener;
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

	const double r_min = 20.0;
	const double r_max = 450.0;
	const double r_factor = ((r_min - r_max) * (r_min - r_max));
	const double pan_width = 100.0;
	double pan, dist;

	for (i = 0; i < ctx->sources_len; ++i) {
		if (!ctx->sources[i]._3d) {
			ctx->sources[i].ampl = 1.0f;
			ctx->sources[i].ampr = 1.0f;
			continue;
		}

		vec3 d = { 0 };
		memcpy(d, ctx->sources[i].pos, sizeof(vec3));
		vec_sub(d, ctx->listener.pos);
		float d_len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);

		if (d_len > r_max) {
			dist = 0.0;
		} else if (d_len < r_min) {
			dist = 1.0;
		} else {
			d_len -= r_max;
			dist = (d_len * d_len) / r_factor;
		}

		struct pointf relative_2d_pos = { d[0], d[2] };
		rotate_pointf(&relative_2d_pos, &(struct pointf) { 0, 0 }, ctx->listener.angle);

		if (relative_2d_pos.x < -pan_width) {
			pan = 0.0;
		} else if (relative_2d_pos.x > pan_width) {
			pan = 1.0;
		} else {
			pan = (relative_2d_pos.x + pan_width) / (pan_width * 2);
		}

		ctx->sources[i].ampr = ctx->sources[i].amp * dist * pan;
		ctx->sources[i].ampl = ctx->sources[i].amp * dist * (1.0 - pan);
	}
}

static float
pct_volume_to_loudness(float pct)
{
	return 10.0f * (logf(1.0f + (pct / 100.0f)) / logf(10.0f));
}

static void
process_messages(struct sound_ctx *sctx, struct write_ctx *wctx)
{
	struct sound_msg *msg;

	while ((msg = ring_buffer_pop(&sctx->ctl))) {
		switch (msg->type) {
		case sound_msg_stop_all:
			wctx->sources_len = 0;
			break;
		case sound_msg_add:
			add_source(sctx, wctx, (vec3) { 0, 0, 0 }, msg->data.add.asset, msg->data.add.flags, false);
			break;
		case sound_msg_add_3d:
			add_source(sctx, wctx, msg->data.add_3d.pos, msg->data.add_3d.asset, msg->data.add_3d.flags, true);
			break;
		case sound_msg_listener:
			memcpy(wctx->listener.pos, msg->data.listener.pos, sizeof(vec3));
			wctx->listener.angle = msg->data.listener.angle;
			break;
		case sound_msg_set_val:
			switch (msg->data.set_val.what) {
			case sound_volume_master:
				wctx->vol.master = pct_volume_to_loudness(msg->data.set_val.val);
				break;
			case sound_volume_music:
				wctx->vol.music = pct_volume_to_loudness(msg->data.set_val.val);
				break;
			case sound_volume_sfx:
				wctx->vol.sfx = pct_volume_to_loudness(msg->data.set_val.val);
				break;
			}
			break;
		}
	}
}

#define FRAMES_MAX 512

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

	if (frames > FRAMES_MAX) {
		frames = FRAMES_MAX;
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

				sl *= wctx.sources[i].ampl;
				sr *= wctx.sources[i].ampr;

				switch (audio_asset_type[wctx.sources[i].asset]) {
				case audio_asset_type_music:
					sl *= wctx.vol.music;
					sr *= wctx.vol.music;
					break;
				case audio_asset_type_sfx:
					sl *= wctx.vol.sfx;
					sr *= wctx.vol.sfx;
					break;
				}

				samples[0] += sl;
				samples[1] += sr;

				wctx.sources[i].bufi += wctx.sources[i].speed;
			}


			ctx->write_sample(areas[0].ptr, samples[0] * wctx.vol.master);
			areas[0].ptr += areas[0].step;

			ctx->write_sample(areas[1].ptr, samples[1] * wctx.vol.master);
			areas[1].ptr += areas[1].step;
		}

		if ((err = soundio_outstream_end_write(outstream))
		    && err != SoundIoErrorUnderflow) {
			LOG_W(log_sound, "stream error: %s", soundio_strerror(err));
			break;
		}
	}
}

void
sc_stop_all(struct sound_ctx *ctx)
{
	ring_buffer_push(&ctx->ctl, &(struct sound_msg){
		.type = sound_msg_stop_all,
	});
}

void
sc_trigger(struct sound_ctx *ctx, enum audio_asset asset, enum audio_flags flags)
{
	ring_buffer_push(&ctx->ctl, &(struct sound_msg){
		.type = sound_msg_add,
		.data.add = {
			.asset = asset,
			.flags = flags,
		}
	});
}

void
sc_trigger_3d(struct sound_ctx *ctx, vec3 pos, enum audio_asset asset, enum audio_flags flags)
{
	ring_buffer_push(&ctx->ctl, &(struct sound_msg){
		.type = sound_msg_add_3d,
		.data.add_3d = {
			.asset = asset,
			.pos = { pos[0], pos[1], pos[2] },
			.flags = flags,
		}
	});
}

void
sc_set_val(struct sound_ctx *ctx, enum sound_val what, float val)
{
	ring_buffer_push(&ctx->ctl, &(struct sound_msg){
		.type = sound_msg_set_val,
		.data.set_val = { .what = what, .val = val }
	});
}

void
sc_update(struct sound_ctx *ctx, vec3 listener, float angle)
{
	ring_buffer_push(&ctx->ctl, &(struct sound_msg){
		.type = sound_msg_listener,
		.data.listener = {
			.pos = { listener[0], listener[1], listener[2] },
			.angle = angle,
		}
	});
}
