#include "posix.h"

#include <stdbool.h>
#include <stdint.h>

#include "client/cfg/common.h"
#include "client/cfg/graphics.h"
#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/ui.h"
#include "shared/util/log.h"

struct colors_t colors = { 0 };

static vec4 grayscale_colors[] = {
	[0]  = { 0.0703125, 0.0703125, 0.0703125, 1.0 },
	[1]  = { 0.109375, 0.109375, 0.109375, 1.0 },
	[2]  = { 0.1484375, 0.1484375, 0.1484375, 1.0 },
	[3]  = { 0.1875, 0.1875, 0.1875, 1.0 },
	[4]  = { 0.2265625, 0.2265625, 0.2265625, 1.0 },
	[5]  = { 0.265625, 0.265625, 0.265625, 1.0 },
	[6]  = { 0.3046875, 0.3046875, 0.3046875, 1.0 },
	[7]  = { 0.34375, 0.34375, 0.34375, 1.0 },
	[8]  = { 0.3828125, 0.3828125, 0.3828125, 1.0 },
	[9]  = { 0.421875, 0.421875, 0.421875, 1.0 },
	[10] = { 0.4609375, 0.4609375, 0.4609375, 1.0 },
	[11] = { 0.5, 0.5, 0.5, 1.0 },
	[12] = { 0.5390625, 0.5390625, 0.5390625, 1.0 },
	[13] = { 0.578125, 0.578125, 0.578125, 1.0 },
	[14] = { 0.6171875, 0.6171875, 0.6171875, 1.0 },
	[15] = { 0.65625, 0.65625, 0.65625, 1.0 },
	[16] = { 0.6953125, 0.6953125, 0.6953125, 1.0 },
	[17] = { 0.734375, 0.734375, 0.734375, 1.0 },
	[18] = { 0.7734375, 0.7734375, 0.7734375, 1.0 },
	[19] = { 0.8125, 0.8125, 0.8125, 1.0 },
	[20] = { 0.8515625, 0.8515625, 0.8515625, 1.0 },
	[21] = { 0.890625, 0.890625, 0.890625, 1.0 },
	[22] = { 0.9296875, 0.9296875, 0.9296875, 1.0 },
};

static void
convert_color(short termclr, float *r, float *g, float *b)
{
	if (termclr >= 233) {
		termclr -= 233;
		*r = grayscale_colors[termclr][0];
		*g = grayscale_colors[termclr][1];
		*b = grayscale_colors[termclr][2];
	} else {
		termclr -= 16;

		*r = ((termclr / 36)    ) * 0.16666666666666666;
		*g = ((termclr % 36) / 6) * 0.16666666666666666;
		*b = ((termclr % 36) % 6) * 0.16666666666666666;
	}
}

static bool
setup_color(void *_, int32_t sect, int32_t type,
	char c, short fg, short bg, short attr, short zi)
{
	float r, g, b;

	convert_color(fg, &r, &g, &b);

	switch (sect) {
	case gfx_cfg_section_entities:
		colors.ent[type][0] = r;
		colors.ent[type][1] = g;
		colors.ent[type][2] = b;
		colors.ent[type][3] = 1.0;
		break;
	case gfx_cfg_section_tiles:
		colors.tile[type][0] = r;
		colors.tile[type][1] = g;
		colors.tile[type][2] = b;
		colors.tile[type][3] = type <= tile_water ? 0.45 : 1.0;
		break;
	}

	return true;
}

bool
color_cfg(char *file)
{
	struct parse_graphics_ctx cfg_ctx = { NULL, setup_color };

	//glUseProgram(ctx->chunks.id);

	if (parse_cfg_file(file, &cfg_ctx, parse_graphics_handler)) {
		//glUniform4fv(ctx->chunks.uni.clr, tile_count, (float *)colors.tile);
		return true;
	} else {
		return false;
	}
}
