#include "posix.h"

#include <stddef.h>

#include "shared/sound/sound.h"

#ifdef HAVE_SOUND
#include "shared/sound/core.h"
#endif

struct sound_ctx *
sound_init(void)
{
#ifdef HAVE_SOUND
	return sc_init();
#else
	return NULL;
#endif
}

void
sound_trigger(struct sound_ctx *ctx, vec3 pos, double pitch)
{
#ifdef HAVE_SOUND
	if (ctx) {
		sc_trigger(ctx, pos, pitch);
	}
#endif
}

void
sound_update(struct sound_ctx *ctx, vec3 listener)
{
#ifdef HAVE_SOUND
	if (ctx) {
		sc_update(ctx, listener);
	}
#endif
}

void
sound_deinit(struct sound_ctx *ctx)
{
#ifdef HAVE_SOUND
	if (ctx) {
		sc_deinit(ctx);
	}
#endif
}
