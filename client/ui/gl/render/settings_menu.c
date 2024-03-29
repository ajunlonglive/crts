#include "posix.h"

#include <stdio.h>

#include "client/ui/gl/render/settings_menu.h"
#include "shared/sound/sound.h"
#include "shared/ui/gl/menu.h"

void
settings_menu(struct client_opts *opts)
{
	const float w = 24;
	{
		static struct menu_slider_ctx slider = {
			.label = "master volume", .unit = "%",
			.w = w,
			.min = 0.0f, .max = 100.0f
		};

		menu_slider(&slider, &opts->sound.master);

		sound_set_val(sound_volume_master, opts->sound.master);
	}

	menu_newline();

	menu_align(w + 1);
	menu.center = false;

	{
		static struct menu_slider_ctx slider = {
			.label = "music", .unit = "%",
			.w = (w - 2) / 2,
			.min = 0.0f, .max = 100.0f
		};

		menu_slider(&slider, &opts->sound.music);

		sound_set_val(sound_volume_music, opts->sound.music);
	}

	++menu.x;

	{
		static struct menu_slider_ctx slider = {
			.label = "sfx", .unit = "%",
			.w = (w - 2) / 2,
			.min = 0.0f, .max = 100.0f
		};

		menu_slider(&slider, &opts->sound.sfx);

		sound_set_val(sound_volume_sfx, opts->sound.sfx);
	}

	menu.center = true;
	menu_newline();

	{
		static char buf[512] = { 0 };
		static struct menu_button_ctx button = {
			.str = buf,
		};

		if (!buf[0]) {
			button.w = snprintf(buf, 512, "%s", sound_device_name(opts->sound.device));
			if (button.w < w + 1) {
				button.w = w + 1;
			}
		}

		if (menu_button_c(&button)) {
			if ((uint32_t)(++opts->sound.device) >= sound_device_output_count()) {
				opts->sound.device = 0;
			}
			buf[0] = 0;

			sound_reset_device(opts->sound.device);
		}
	}


	menu_newline();

	{
		static struct menu_slider_ctx mouse_slider = {
			.label = "mouse sens.", .unit = "%",
			.w = w,
			.min = 0.0f, .max = 100.0f
		};

		menu_slider(&mouse_slider, &opts->ui_cfg.mouse_sensitivity);
	}

	menu_newline();

	{
		static struct menu_slider_ctx mouse_slider = {
			.label = "ui scale", .unit = "X",
			.w = w,
			.min = 15.0f, .max = 30.0f,
			.step = 1.0,
		};

		if (opts->ui_cfg.ui_scale == 0.0) {
			opts->ui_cfg.ui_scale = menu.scale;
		}

		if (menu_slider(&mouse_slider, &opts->ui_cfg.ui_scale)) {
			menu_set_scale(opts->ui_cfg.ui_scale);
		}
	}

	menu_newline();
}

