#include "posix.h"

#include <stddef.h>

#include "shared/sound/sound.h"

#ifdef HAVE_SOUND
#include "shared/sound/core.h"
#include "shared/sound/setup.h"
#endif

bool
sound_init(struct sound_ctx *ctx)
{
#ifdef HAVE_SOUND
	return ctx->enabled = sc_init(ctx);
#else
	return false;
#endif
}


void
sound_trigger(struct sound_ctx *ctx, vec3 pos, enum audio_asset asset,
	enum audio_flags flags)
{
#ifdef HAVE_SOUND
	if (ctx->enabled) {
		sc_trigger(ctx, pos, asset, flags);
	}
#endif
}

void
sound_update(struct sound_ctx *ctx, vec3 listener)
{
#ifdef HAVE_SOUND
	if (ctx->enabled) {
		sc_update(ctx, listener);
	}
#endif
}

void
sound_deinit(struct sound_ctx *ctx)
{
#ifdef HAVE_SOUND
	if (ctx->enabled) {
		sc_deinit(ctx);
	}
#endif
}
