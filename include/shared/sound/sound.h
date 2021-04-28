#ifndef SHARED_SOUND_SOUND_H
#define SHARED_SOUND_SOUND_H

#include "shared/math/linalg.h"
#include "shared/util/file_formats/wav.h"

enum audio_flags {
	audio_flag_loop = 1 << 0,
	audio_flag_rand = 1 << 1,
};

enum audio_asset {
	audio_asset_theme,
	audio_asset_step_dirt,
	audio_asset_step_grass,
	audio_asset_step_rock,
	audio_asset_step_sand,
	audio_asset_die,
	audio_asset_spawn,
	audio_asset_count,
};

struct sound_ctx {
	struct wav assets[audio_asset_count];
	struct SoundIo *soundio;
	struct SoundIoDevice *device;
	struct SoundIoOutStream *outstream;
	struct SoundIoRingBuffer *buf;
	void (*write_sample)(char *ptr, double sample);
	bool enabled;
};

bool sound_init(struct sound_ctx *ctx);
void sound_update(struct sound_ctx *ctx, vec3 listener);
void sound_trigger(struct sound_ctx *ctx, vec3 pos, enum audio_asset asset,
	enum audio_flags flags);
void sound_deinit(struct sound_ctx *ctx);
#endif
