#include "posix.h"

#include <stddef.h>

#include "shared/sound/sound.h"
#include "shared/util/log.h"

#ifdef HAVE_SOUND
#include "shared/sound/core.h"
#include "shared/sound/setup.h"
#endif

static struct sound_ctx sound_ctx;

uint32_t
sound_device_output_count(void)
{
#ifdef HAVE_SOUND
	return sound_ctx.output_count;
#else
	return 0;
#endif
}

const char *
sound_device_name(uint32_t device)
{
#ifdef HAVE_SOUND
	return sc_device_name(&sound_ctx, device);
#else
	return NULL;
#endif
}

bool
sound_list_devices(void)
{
#ifdef HAVE_SOUND
	return sc_list_devices();
#else
	return false;
#endif
}

bool
sound_reset_device(uint32_t device)
{
#ifdef HAVE_SOUND
	sound_deinit();
	return sound_init(device);
#else
	return false;
#endif
}

bool
sound_init(uint32_t device)
{
	if (sound_ctx.initialized) {
		return true;
	} else {
		sound_ctx.initialized = true;
	}

#ifdef HAVE_SOUND
	return sound_ctx.enabled = sc_init(&sound_ctx, device);
#else
	return false;
#endif
}

void
sound_trigger_3d(vec3 pos, enum audio_asset asset, enum audio_flags flags)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_trigger_3d(&sound_ctx, pos, asset, flags);
	}
#endif
}

void
sound_trigger(enum audio_asset asset, enum audio_flags flags)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_trigger(&sound_ctx, asset, flags);
	}
#endif
}

void
sound_stop_all(void)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_stop_all(&sound_ctx);
	}
#endif
}

void
sound_set_val(enum sound_val what, float val)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_set_val(&sound_ctx, what, val);
	}
#endif
}

void
sound_update(vec3 listener, float angle)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_update(&sound_ctx, listener, angle);
	}
#endif
}

void
sound_deinit(void)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		L(log_sound, "deinitializing sound");
		sc_deinit(&sound_ctx);
		sound_ctx = (struct sound_ctx) { 0 };
	}
#endif
}
