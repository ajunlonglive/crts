#ifndef SHARED_SOUND_CORE_H
#define SHARED_SOUND_CORE_H
#include <stdbool.h>

#include "shared/sound/sound.h"
#include "shared/types/ring_buffer.h"

struct sound_ctx {
	struct wav assets[audio_asset_count];
	struct ring_buffer ctl;
	struct SoundIo *soundio;
	struct SoundIoDevice *device;
	struct SoundIoOutStream *outstream;
	void (*write_sample)(char *ptr, double sample);
	float vol;
	bool enabled;
};

enum sound_msg_type {
	sound_msg_add,
	sound_msg_add_3d,
	sound_msg_listener,
	sound_msg_stop_all,
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

void sc_update(struct sound_ctx *ctx, vec3 listener);
void sc_trigger(struct sound_ctx *ctx, enum audio_asset asset, enum audio_flags flags);
void sc_trigger_3d(struct sound_ctx *ctx, vec3 pos, enum audio_asset asset, enum audio_flags flags);
void sc_stop_all(struct sound_ctx *ctx);
void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
#endif
