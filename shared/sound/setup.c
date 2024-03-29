#include "posix.h"

#include <soundio/soundio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "shared/sound/core.h"
#include "shared/sound/setup.h"
#include "shared/sound/sound.h"
#include "shared/util/file_formats/wav.h"
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
	LOG_W(log_sound, "underflow %d", count++);
}

static bool
load_assets(struct sound_ctx *ctx)
{
	static const char *asset_name[audio_asset_count] = {
		[audio_asset_theme_1]    = "theme_1.wav",
		[audio_asset_theme_2]    = "theme_2.wav",
		[audio_asset_theme_3]    = "theme_3.wav",
		[audio_asset_step_dirt]  = "step_dirt.wav",
		[audio_asset_step_grass] = "step_grass.wav",
		[audio_asset_step_rock]  = "step_rock.wav",
		[audio_asset_step_sand]  = "step_sand.wav",
		[audio_asset_die]        = "die.wav",
		[audio_asset_spawn]      = "spawn.wav",
	};

	uint32_t i;
	for (i = 0; i < audio_asset_count; ++i) {
		if (!load_wav(asset_name[i], &ctx->assets[i])) {
			LOG_W(log_sound, "failed to load '%s'", asset_name[i]);
		}
	}

	return true;
}

const char *
sc_device_name(struct sound_ctx *ctx, uint32_t device)
{
	if (!ctx->initialized) {
		return "audio disabled";
	}

	static char buf[1024];
	struct SoundIoDevice *d = soundio_get_output_device(ctx->soundio, device);

	if (!d) {
		return NULL;
	}

	strncpy(buf, d->name, 1023);

	soundio_device_unref(d);
	return buf;
}

bool
sc_list_devices(void)
{
	uint32_t i, output_count;
	struct SoundIo *soundio;
	struct SoundIoDevice *device;

	if (!(soundio = soundio_create())) {
		return false;
	}

	soundio_connect(soundio);
	soundio_flush_events(soundio);

	output_count = soundio_output_device_count(soundio);
	for (i = 0; i < output_count; ++i) {
		device = soundio_get_output_device(soundio, i);

		printf("%2d | %s\n", i, device->name);

		soundio_device_unref(device);
	}

	soundio_destroy(soundio);
	return true;
}

static bool
sc_init_device(struct sound_ctx *ctx, int32_t device)
{
	enum SoundIoBackend backend = SoundIoBackendNone;
	int err;

	if (!(ctx->soundio = soundio_create())) {
		return false;
	}

	if (backend == SoundIoBackendNone) {
		soundio_connect(ctx->soundio);
	} else {
		soundio_connect_backend(ctx->soundio, backend);
	}
	backend = ctx->soundio->current_backend;

	L(log_sound, "sound backend: %s", soundio_backend_name(backend));

	soundio_flush_events(ctx->soundio);

	ctx->output_count = soundio_output_device_count(ctx->soundio);

	int selected_device_index;

	if (device <= -1) {
		selected_device_index = soundio_default_output_device_index(ctx->soundio);
	} else {
		selected_device_index = device;
	}

	if (selected_device_index < 0) {
		L(log_sound, "Output device not found");
		return false;
	}

	ctx->device = soundio_get_output_device(ctx->soundio, selected_device_index);
	if (!ctx->device) {
		L(log_sound, "out of memory");
		return false;
	}

	L(log_sound, "Output device: %s", ctx->device->name);

	if (ctx->device->probe_error) {
		L(log_sound, "Cannot probe device: %s", soundio_strerror(ctx->device->probe_error));
		return false;
	}

	if (!(ctx->outstream = soundio_outstream_create(ctx->device))) {
		L(log_sound, "out of memory");
		return false;
	}

	ring_buffer_init(&ctx->ctl, sizeof(struct sound_msg), 256);

	ctx->outstream->write_callback = write_callback;
	ctx->outstream->underflow_callback = underflow_callback;
	ctx->outstream->name = NULL;

	switch (backend) {
	case SoundIoBackendAlsa:
		// if this is set to 0, libsoundio sets it to 1!
		ctx->outstream->software_latency = 0.1;
		break;
	default:
		ctx->outstream->software_latency = 0.0;
		break;
	}

	ctx->outstream->sample_rate = 0;
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
		L(log_sound, "no suitable device format available.");
		return false;
	}

	if ((err = soundio_outstream_open(ctx->outstream))) {
		L(log_sound, "unable to open device: %s", soundio_strerror(err));
		return false;
	}

	L(log_sound, "software latency: %f", ctx->outstream->software_latency);

	if (ctx->outstream->layout_error) {
		L(log_sound, "unable to set channel layout: %s", soundio_strerror(ctx->outstream->layout_error));
	}

	if ((err = soundio_outstream_start(ctx->outstream))) {
		L(log_sound, "unable to start device: %s", soundio_strerror(err));
		return false;
	}

	return true;
}

bool
sc_init(struct sound_ctx *ctx, int32_t device)
{
	return sc_init_device(ctx, device) && load_assets(ctx);

}

bool
sc_reset(struct sound_ctx *ctx, int32_t device)
{
	soundio_outstream_destroy(ctx->outstream);
	soundio_device_unref(ctx->device);
	ring_buffer_deinit(&ctx->ctl);
	soundio_destroy(ctx->soundio);

	return sc_init_device(ctx, device);
}

void
sc_deinit(struct sound_ctx *ctx)
{
	soundio_outstream_destroy(ctx->outstream);
	soundio_device_unref(ctx->device);
	ring_buffer_deinit(&ctx->ctl);
	soundio_destroy(ctx->soundio);
}
