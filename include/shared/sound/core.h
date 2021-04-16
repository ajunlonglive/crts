#ifndef SHARED_SOUND_CORE_H
#define SHARED_SOUND_CORE_H
#include <stdbool.h>

#include "shared/sound/sound.h"

void sc_update(struct sound_ctx *ctx, vec3 listener);
void sc_trigger(struct sound_ctx *ctx, vec3 pos, float pitch);
void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
#endif
