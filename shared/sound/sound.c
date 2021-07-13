#include "posix.h"

#include <stddef.h>

#include "shared/sound/sound.h"

#ifdef HAVE_SOUND
#include "shared/sound/core.h"
#include "shared/sound/setup.h"
#endif

static struct sound_ctx sound_ctx;

bool
sound_init(void)
{
	static bool initialized = false;

	if (initialized) {
		return true;
	} else {
		initialized = true;
	}

#ifdef HAVE_SOUND
	return sound_ctx.enabled = sc_init(&sound_ctx);
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
sound_update(vec3 listener)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_update(&sound_ctx, listener);
	}
#endif
}

void
sound_deinit(void)
{
#ifdef HAVE_SOUND
	if (sound_ctx.enabled) {
		sc_deinit(&sound_ctx);
	}
#endif
}
