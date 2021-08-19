#ifndef SHARED_SOUND_SETUP_H
#define SHARED_SOUND_SETUP_H
#include <stdbool.h>

#include "shared/sound/core.h"

bool sc_list_devices(void);
bool sc_init(struct sound_ctx *ctx, int32_t device);
void sc_deinit(struct sound_ctx *ctx);
#endif
