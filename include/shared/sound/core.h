#ifndef SHARED_SOUND_CORE_H
#define SHARED_SOUND_CORE_H
#include <stdbool.h>

#include "shared/sound/sound.h"

struct sound_ctx {
	struct wav assets[audio_asset_count];
	struct SoundIo *soundio;
	struct SoundIoDevice *device;
	struct SoundIoOutStream *outstream;
	struct SoundIoRingBuffer *buf;
	void (*write_sample)(char *ptr, double sample);
	float vol;
	bool enabled;
};

void sc_update(struct sound_ctx *ctx, vec3 listener);
void sc_trigger(struct sound_ctx *ctx, enum audio_asset asset, enum audio_flags flags);
void sc_trigger_3d(struct sound_ctx *ctx, vec3 pos, enum audio_asset asset, enum audio_flags flags);
void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
#endif
