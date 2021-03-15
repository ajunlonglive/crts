#ifndef SHARED_SOUND_SOUND_H
#define SHARED_SOUND_SOUND_H

#include "shared/math/linalg.h"

struct sound_ctx;

struct sound_ctx *sound_init(void);
void sound_update(struct sound_ctx *ctx, vec3 listener);
void sound_trigger(struct sound_ctx *ctx, vec3 pos, double pitch);
void sound_deinit(struct sound_ctx *ctx);
#endif
