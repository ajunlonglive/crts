#ifndef SHARED_SOUND_SOUND_H
#define SHARED_SOUND_SOUND_H

#include "shared/math/linalg.h"
#include "shared/util/file_formats/wav.h"

enum audio_flags {
	audio_flag_loop = 1 << 0,
	audio_flag_rand = 1 << 1,
};

enum audio_asset {
	audio_asset_theme_1,
	audio_asset_theme_2,
	audio_asset_theme_3,
	audio_asset_step_dirt,
	audio_asset_step_grass,
	audio_asset_step_rock,
	audio_asset_step_sand,
	audio_asset_die,
	audio_asset_spawn,
	audio_asset_count,
};

bool sound_init(void);
void sound_update(vec3 listener);
void sound_trigger_3d(vec3 pos, enum audio_asset asset, enum audio_flags flags);
void sound_deinit(void);
#endif
