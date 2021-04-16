#ifndef SHARED_SOUND_SOUND_H
#define SHARED_SOUND_SOUND_H

#include "shared/math/linalg.h"

struct sound_ctx {
	struct SoundIo *soundio;
	struct SoundIoDevice *device;
	struct SoundIoOutStream *outstream;
	struct SoundIoRingBuffer *buf;
	void (*write_sample)(char *ptr, double sample);
	bool enabled;
};

bool sound_init(struct sound_ctx *ctx);
void sound_update(struct sound_ctx *ctx, vec3 listener);
void sound_trigger(struct sound_ctx *ctx, vec3 pos, float pitch);
void sound_deinit(struct sound_ctx *ctx);
#endif
