#ifndef CLIENT_UI_OPENGL_RENDER_WATER_H
#define CLIENT_UI_OPENGL_RENDER_WATER_H

#include "client/ui/opengl/ui.h"

struct water_fx {
	uint32_t reflect_w,   refract_w,
		 reflect_h,   refract_h,
		 reflect_fb,  refract_fb,
		 reflect_tex, refract_tex,
		 reflect_db,  refract_dtex;
};

bool render_world_setup_water(struct water_fx *wfx);
void render_water_setup_frame(struct opengl_ui_ctx *ctx);
void render_water(struct opengl_ui_ctx *ctx, struct water_fx *wfx);
#endif
