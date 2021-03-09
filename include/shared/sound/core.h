#ifndef SHARED_SOUND_CORE_H
#define SHARED_SOUND_CORE_H
#include <stdbool.h>

struct sound_ctx {
	struct SoundIo *soundio;
	struct SoundIoDevice *device;
	struct SoundIoOutStream *outstream;
	struct SoundIoRingBuffer *buf;
};

struct sound_ctx *sc_init(void);
void sc_update(struct sound_ctx *ctx);
void sc_trigger(struct sound_ctx *ctx);
void sc_deinit(struct sound_ctx *ctx);
#endif
