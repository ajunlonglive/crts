#ifndef SHARED_SOUND_SETUP_H
#define SHARED_SOUND_SETUP_H
#include <stdbool.h>

#include "shared/sound/core.h"

bool sc_list_devices(void);
const char *sc_device_name(struct sound_ctx *ctx, uint32_t device);
bool sc_init(struct sound_ctx *ctx, int32_t device);
void sc_deinit(struct sound_ctx *ctx);
#endif
