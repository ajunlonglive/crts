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

enum sound_val {
	sound_volume_master,
	sound_volume_music,
	sound_volume_sfx,
};

bool sound_init(void);
void sound_update(vec3 listener);
void sound_trigger_3d(vec3 pos, enum audio_asset asset, enum audio_flags flags);
void sound_trigger(enum audio_asset asset, enum audio_flags flags);
void sound_set_val(enum sound_val what, float val);
void sound_stop_all(void);
void sound_deinit(void);
#endif
