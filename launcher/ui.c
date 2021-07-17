#include "posix.h"

#include <glad/gl.h>
#include <stdlib.h>

#include "shared/input/mouse.h"
#include "shared/sound/sound.h"
#include "shared/ui/gl/menu.h"
#include "shared/ui/gl/window.h"
#include "launcher/ui.h"

enum launcher_button {
	lb_singleplayer,
	lb_multiplayer,
	lb_settings,
	lb_join,
	lb_host,
	lb_quit,
	lb_back,
	launcher_button_count
};

static struct menu_button_ctx buttons[launcher_button_count] = {
	[lb_singleplayer] = { .str = "singleplayer" },
	[lb_multiplayer] = { .str = "multiplayer" },
	[lb_settings] = { .str = "settings" },
	[lb_join] = { .str = "join" },
	[lb_host] = { .str = "host" },
	[lb_quit] = { .str = "quit" },
	[lb_back] = { .str = "back" },
};

enum launcher_menu {
	launcher_menu_main,
	launcher_menu_multiplayer,
	launcher_menu_settings,
};

static enum launcher_menu cur_menu;

static void
hover_sound_cb(void)
{
	sound_trigger(audio_asset_step_sand, 0);
}

static void
click_sound_cb(void)
{
	sound_trigger(audio_asset_step_rock, 0);
}

static void
key_input_cb(void *_ctx, uint8_t mod, uint8_t key, uint8_t act)
{
	struct launcher_ui_ctx *ctx = _ctx;

	menu_handle_input(&ctx->menu, key);
}

void
launcher_ui_init(struct launcher_ui_ctx *ctx)
{
	*ctx = (struct launcher_ui_ctx) { .win = gl_win_init(ctx), };
	ctx->win->key_input_callback = key_input_cb;

	if (!menu_setup(&ctx->menu)) {
		return;
	}

	if (!sound_init()) {
		return;
	}

	sound_stop_all();

	gl_win_set_cursor_display(true);
	glClearColor(0, 0, 0, 1.0);

	ctx->menu.scale = 20;
	ctx->menu.button_pad = 2.0f;
	ctx->menu.center = true;
	ctx->menu.linesep = 2.4f;

	uint32_t i;
	for (i = 0; i < launcher_button_count; ++i) {
		buttons[i] = (struct menu_button_ctx) {
			.w = 15.0f,
			.str = buttons[i].str,
			.hover_cb = hover_sound_cb,
			.click_cb = click_sound_cb,
		};
	}
}

static void
settings_menu(struct menu_ctx *menu)
{
	{
		static float vol = 100.0f;
		static struct menu_slider_ctx slider = {
			.label = "volume", .unit = "%",
			.w = 20,
			.min = 0.0f, .max = 100.0f
		};

		menu_slider(menu, &slider, &vol);
		menu_newline(menu);
	}

	{
		static float val = 50.0f;
		static struct menu_slider_ctx mouse_slider = {
			.label = "mouse sens.", .unit = "%",
			.w = 20,
			.min = 0.0f, .max = 100.0f
		};

		menu_slider(menu, &mouse_slider, &val);
		menu_newline(menu);
	}

	{
		static float scale = 0.0;
		static struct menu_slider_ctx mouse_slider = {
			.label = "ui scale", .unit = "X",
			.w = 20,
			.min = 15.0f, .max = 30.0f,
			.step = 1.0,
		};

		if (scale == 0.0) {
			scale = menu->scale;
		}

		if (menu_slider(menu, &mouse_slider, &scale)) {
			menu_set_scale(menu, scale);
		}

		menu_newline(menu);
	}
}

static bool
launcher_button(struct launcher_ui_ctx *ctx, enum launcher_button button)
{
	return menu_button_c(&ctx->menu, &buttons[button]);
}

static void
lm_settings(struct launcher_ui_ctx *ctx)
{
	menu_str(&ctx->menu, "settings");
	menu_newline(&ctx->menu);

	settings_menu(&ctx->menu);

	if (launcher_button(ctx, lb_back)) {
		cur_menu = launcher_menu_main;
	}
	menu_newline(&ctx->menu);
}

static void
lm_multiplayer(struct launcher_ui_ctx *ctx)
{
	menu_str(&ctx->menu, "multiplayer");
	menu_newline(&ctx->menu);

	if (launcher_button(ctx, lb_join)) {
	}
	menu_newline(&ctx->menu);

	{
		static struct menu_textbox_ctx textbox = {
			.buf = "127.0.0.1"
		};
		menu_textbox(&ctx->menu, &textbox);
		menu_newline(&ctx->menu);
	}

	if (launcher_button(ctx, lb_host)) {
	}
	menu_newline(&ctx->menu);

	if (launcher_button(ctx, lb_back)) {
		cur_menu = launcher_menu_main;
	}
	menu_newline(&ctx->menu);
}

static void
lm_main(struct launcher_ui_ctx *ctx)
{
	menu_str(&ctx->menu, "cube chaos");
	menu_newline(&ctx->menu);

	if (launcher_button(ctx, lb_singleplayer)) {
		ctx->stop = true;
	}
	menu_newline(&ctx->menu);

	if (launcher_button(ctx, lb_multiplayer)) {
		cur_menu = launcher_menu_multiplayer;
	}
	menu_newline(&ctx->menu);

	if (launcher_button(ctx, lb_settings)) {
		cur_menu = launcher_menu_settings;
	}
	menu_newline(&ctx->menu);

	if (launcher_button(ctx, lb_quit)) {
		exit(0);
	}
	menu_newline(&ctx->menu);
}

void
launcher_ui_render(struct launcher_ui_ctx *ctx)
{
	gl_win_poll_events(ctx);

	glViewport(0, 0, ctx->win->px_width, ctx->win->px_height);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	menu_begin(&ctx->menu, ctx->win, ctx->win->mouse.x, ctx->win->mouse.y, ctx->win->mouse.buttons & mb_1);

	ctx->menu.y = 5;

	switch (cur_menu) {
	case launcher_menu_main:
		lm_main(ctx);
		break;
	case launcher_menu_multiplayer:
		lm_multiplayer(ctx);
		break;
	case launcher_menu_settings:
		lm_settings(ctx);
		break;
	}

	menu_render(&ctx->menu, ctx->win);

	gl_win_swap_buffers();
	sound_update((vec3){ 0, 0, 0 });
	ctx->win->resized = false;
}
