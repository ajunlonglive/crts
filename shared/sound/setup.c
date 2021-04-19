#include "posix.h"

#include <soundio/soundio.h>
#include <stddef.h>
#include <stdint.h>

#include "shared/sound/core.h"
#include "shared/sound/setup.h"
#include "shared/sound/sound.h"
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

static void
underflow_callback(struct SoundIoOutStream *outstream)
{
	static int count = 0;
	LOG_W("underflow %d", count++);
}

bool
sc_init(struct sound_ctx *ctx)
{
	enum SoundIoBackend backend = SoundIoBackendNone;
	char *stream_name = NULL;
	double latency = 0.0;
	int sample_rate = 0;
	int err;

	if (!(ctx->soundio = soundio_create())) {
		return false;
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
		return false;
	}

	ctx->device = soundio_get_output_device(ctx->soundio, selected_device_index);
	if (!ctx->device) {
		L("out of memory");
		return false;
	}

	L("Output device: %s", ctx->device->name);

	if (ctx->device->probe_error) {
		L("Cannot probe device: %s", soundio_strerror(ctx->device->probe_error));
		return false;
	}

	if (!(ctx->outstream = soundio_outstream_create(ctx->device))) {
		L("out of memory");
		return false;
	}

	if (!(ctx->buf = soundio_ring_buffer_create(ctx->soundio, 4096 * 8))) {
		L("unable to allocate ring buffer");
		return false;
	}

	ctx->outstream->write_callback = write_callback;
	ctx->outstream->underflow_callback = underflow_callback;
	ctx->outstream->name = stream_name;
	ctx->outstream->software_latency = latency;
	ctx->outstream->sample_rate = sample_rate;
	ctx->outstream->userdata = ctx;

	if (soundio_device_supports_format(ctx->device, SoundIoFormatFloat32NE)) {
		ctx->outstream->format = SoundIoFormatFloat32NE;
		ctx->write_sample = write_sample_float32ne;
	} else if (soundio_device_supports_format(ctx->device, SoundIoFormatFloat64NE)) {
		ctx->outstream->format = SoundIoFormatFloat64NE;
		ctx->write_sample = write_sample_float64ne;
	} else if (soundio_device_supports_format(ctx->device, SoundIoFormatS32NE)) {
		ctx->outstream->format = SoundIoFormatS32NE;
		ctx->write_sample = write_sample_s32ne;
	} else if (soundio_device_supports_format(ctx->device, SoundIoFormatS16NE)) {
		ctx->outstream->format = SoundIoFormatS16NE;
		ctx->write_sample = write_sample_s16ne;
	} else {
		L("No suitable device format available.");
		return false;
	}

	if ((err = soundio_outstream_open(ctx->outstream))) {
		L("unable to open device: %s", soundio_strerror(err));
		return false;
	}

	L("Software latency: %f", ctx->outstream->software_latency);

	if (ctx->outstream->layout_error) {
		L("unable to set channel layout: %s", soundio_strerror(ctx->outstream->layout_error));
	}

	if ((err = soundio_outstream_start(ctx->outstream))) {
		L("unable to start device: %s", soundio_strerror(err));
		return false;
	}

	return true;
}

void
sc_deinit(struct sound_ctx *ctx)
{
	soundio_outstream_destroy(ctx->outstream);
	soundio_device_unref(ctx->device);
	soundio_ring_buffer_destroy(ctx->buf);
	soundio_destroy(ctx->soundio);
}