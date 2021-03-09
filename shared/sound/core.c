#include "posix.h"

#include <math.h>
#include <soundio/soundio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/geom.h"
#include "shared/sound/core.h"
#include "shared/util/log.h"

static void
write_sample_s16ne(char *ptr, double sample)
{
	int16_t *buf = (int16_t *)ptr;
	double range = (double)INT16_MAX - (double)INT16_MIN;
	double val = sample * range / 2.0;
	*buf = val;
}

static void
write_sample_s32ne(char *ptr, double sample)
{
	int32_t *buf = (int32_t *)ptr;
	double range = (double)INT32_MAX - (double)INT32_MIN;
	double val = sample * range / 2.0;
	*buf = val;
}

static void
write_sample_float32ne(char *ptr, double sample)
{
	float *buf = (float *)ptr;
	*buf = sample;
}

static void
write_sample_float64ne(char *ptr, double sample)
{
	double *buf = (double *)ptr;
	*buf = sample;
}

static void (*write_sample)(char *ptr, double sample);

static void
write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
	const struct SoundIoChannelLayout *layout = &outstream->layout;
	struct SoundIoChannelArea *areas;
	int err;
	struct sound_ctx *ctx = outstream->userdata;
	double *buf = (double *)soundio_ring_buffer_read_ptr(ctx->buf);
	double sample;
	uint32_t len = soundio_ring_buffer_fill_count(ctx->buf) / sizeof(double);
	uint32_t framei = 0;
	int32_t tmp;

	assert(frame_count_max >= 0);

	if (!frame_count_max || !len) {
		return;
	}

	uint32_t frames = (uint32_t)frame_count_max > len ? len : (uint32_t)frame_count_max;

	while (framei < frames) {
		tmp = frames - framei;

		if ((err = soundio_outstream_begin_write(outstream, &areas, &tmp))) {
			LOG_W("stream error: %s", soundio_strerror(err));
			break;
		} else if (tmp <= 0) {
			break;
		}

		const uint32_t stop_at = framei + tmp;
		for (; framei < stop_at; ++framei) {
			sample = buf[framei];

			for (int channel = 0; channel < layout->channel_count; channel += 1) {
				write_sample(areas[channel].ptr, sample);
				areas[channel].ptr += areas[channel].step;
			}
		}

		if ((err = soundio_outstream_end_write(outstream))
		    && err != SoundIoErrorUnderflow) {
			LOG_W("stream error: %s", soundio_strerror(err));
			break;
		}
	}

	soundio_ring_buffer_advance_read_ptr(ctx->buf, frames * sizeof(double));
}

static void
underflow_callback(struct SoundIoOutStream *outstream)
{
	static int count = 0;
	LOG_W("underflow %d", count++);
}

static struct sound_ctx sound_ctx;

struct sound_ctx *
sc_init(void)
{
	struct sound_ctx *ctx = &sound_ctx;

	enum SoundIoBackend backend = SoundIoBackendNone;
	char *stream_name = NULL;
	double latency = 0.0;
	int sample_rate = 0;
	int err;

	/* raw = true; */
	/* backend = SoundIoBackendDummy; */
	/* backend = SoundIoBackendAlsa; */
	/* backend = SoundIoBackendPulseAudio; */
	/* backend = SoundIoBackendJack; */
	/* backend = SoundIoBackendCoreAudio; */
	/* backend = SoundIoBackendWasapi; */

	if (!(ctx->soundio = soundio_create())) {
		return NULL;
	}

	if (backend == SoundIoBackendNone) {
		soundio_connect(ctx->soundio);
	} else {
		soundio_connect_backend(ctx->soundio, backend);
	}

	L("sound backend: %s", soundio_backend_name(ctx->soundio->current_backend));

	soundio_flush_events(ctx->soundio);

	int selected_device_index = soundio_default_output_device_index(ctx->soundio);

	if (selected_device_index < 0) {
		L("Output device not found");
		return NULL;
	}

	ctx->device = soundio_get_output_device(ctx->soundio, selected_device_index);
	if (!ctx->device) {
		L("out of memory");
		return NULL;
	}

	L("Output device: %s", ctx->device->name);

	if (ctx->device->probe_error) {
		L("Cannot probe device: %s", soundio_strerror(ctx->device->probe_error));
		return NULL;
	}

	if (!(ctx->outstream = soundio_outstream_create(ctx->device))) {
		L("out of memory");
		return NULL;
	}

	if (!(ctx->buf = soundio_ring_buffer_create(ctx->soundio, 4096 * 16))) {
		L("unable to allocate ring buffer");
		return NULL;
	}

	L("allocated ring buf: %d", soundio_ring_buffer_capacity(ctx->buf));

	ctx->outstream->write_callback = write_callback;
	ctx->outstream->underflow_callback = underflow_callback;
	ctx->outstream->name = stream_name;
	ctx->outstream->software_latency = latency;
	ctx->outstream->sample_rate = sample_rate;
	ctx->outstream->userdata = ctx;

	L("%p", (void *)ctx->outstream);

	if (soundio_device_supports_format(ctx->device, SoundIoFormatFloat32NE)) {
		ctx->outstream->format = SoundIoFormatFloat32NE;
		write_sample = write_sample_float32ne;
	} else if (soundio_device_supports_format(ctx->device, SoundIoFormatFloat64NE)) {
		ctx->outstream->format = SoundIoFormatFloat64NE;
		write_sample = write_sample_float64ne;
	} else if (soundio_device_supports_format(ctx->device, SoundIoFormatS32NE)) {
		ctx->outstream->format = SoundIoFormatS32NE;
		write_sample = write_sample_s32ne;
	} else if (soundio_device_supports_format(ctx->device, SoundIoFormatS16NE)) {
		ctx->outstream->format = SoundIoFormatS16NE;
		write_sample = write_sample_s16ne;
	} else {
		L("No suitable device format available.");
		return NULL;
	}

	if ((err = soundio_outstream_open(ctx->outstream))) {
		L("unable to open device: %s", soundio_strerror(err));
		return NULL;
	}

	L("Software latency: %f", ctx->outstream->software_latency);

	if (ctx->outstream->layout_error) {
		L("unable to set channel layout: %s", soundio_strerror(ctx->outstream->layout_error));
	}

	if ((err = soundio_outstream_start(ctx->outstream))) {
		L("unable to start device: %s", soundio_strerror(err));
		return NULL;
	}

	return ctx;
}

static const double pitch = 440.0;
static double amp = 1.0;
static double seconds_offset = 0.0;

void
sc_trigger(struct sound_ctx *ctx)
{
	amp = 1.0;
}

void
sc_update(struct sound_ctx *ctx)
{
	double *buf = (double *)soundio_ring_buffer_write_ptr(ctx->buf);
	uint32_t len = soundio_ring_buffer_free_count(ctx->buf) / sizeof(double);

	uint32_t i;

	const double float_sample_rate = ctx->outstream->sample_rate;
	const double seconds_per_frame = 1.0 / float_sample_rate;
	const double radians_per_second = pitch * 2.0 * 3.14159265358979323846264338328; /* maybe PI needs to be more precise? */

	if (!len) {
		return;
	}

	for (i = 0; i < len; ++i) {
		buf[i] = sin((seconds_offset + i * seconds_per_frame) * radians_per_second) * amp;
		if (amp > 0.0) {
			amp -= 0.0005;
		}
	}
	seconds_offset = fmod(seconds_offset + seconds_per_frame * len, 1.0);

	soundio_ring_buffer_advance_write_ptr(ctx->buf, len * sizeof(double));
}

void
sc_deinit(struct sound_ctx *ctx)
{
	soundio_outstream_destroy(ctx->outstream);
	soundio_device_unref(ctx->device);
	soundio_ring_buffer_destroy(ctx->buf);
	soundio_destroy(ctx->soundio);
}
