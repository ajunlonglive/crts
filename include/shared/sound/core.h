#ifndef SHARED_SOUND_CORE_H
#define SHARED_SOUND_CORE_H
#include <stdbool.h>

#include "shared/math/linalg.h"

struct sound_ctx {
	struct SoundIo *soundio;
	struct SoundIoDevice *device;
	struct SoundIoOutStream *outstream;
	struct SoundIoRingBuffer *buf;
	/* vec3 listener; */
};

struct sound_ctx *sc_init(void);
void sc_update(struct sound_ctx *ctx, vec3 listener);
void sc_trigger(struct sound_ctx *ctx, vec3 pos, double pitch);
void sc_deinit(struct sound_ctx *ctx);
#endif
