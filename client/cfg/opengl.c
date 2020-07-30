#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/cfg/opengl.h"
#include "shared/math/geom.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

#define OPENGL_CFG "opengl.ini"

enum opengl_opt {
	opt_shadows,
	opt_shadow_map_res,
	opt_font_scale,
	opt_cam_height_max,
	opt_cam_height_min,
	opt_cam_pitch_max,
	opt_cam_pitch_min,
	opt_cam_yaw,
	opt_water,
};

static struct cfg_lookup_table keys =  {
	"shadows",        opt_shadows,
	"shadow_map_res", opt_shadow_map_res,
	"font_scale",     opt_font_scale,
	"cam_height_max", opt_cam_height_max,
	"cam_height_min", opt_cam_height_min,
	"cam_pitch_max",  opt_cam_pitch_max,
	"cam_pitch_min",  opt_cam_pitch_min,
	"cam_yaw",        opt_cam_yaw,
	"water",          opt_water,
};

static bool
str_to_bool(const char *str)
{
	return strcmp(str, "on") == 0 || strcmp(str, "true") == 0;
}

static float
strdeg_to_rad(const char *str)
{
	return strtof(str, NULL) * PI / 180;
}

static bool
parse_opengl_cfg_handler(void *vp, const char *sec, const char *k,
	const char *v, uint32_t line)
{
	struct opengl_opts *opts = vp;

	switch (cfg_string_lookup(k, &keys)) {
	case opt_shadows:
		opts->shadows = str_to_bool(v);
		break;
	case opt_shadow_map_res:
		opts->shadow_map_res = strtoul(v, NULL, 10);
		break;
	case opt_font_scale:
		opts->font_scale = strtof(v, NULL);
		break;
	case opt_cam_height_max:
		opts->cam_height_max = strtof(v, NULL);
		break;
	case opt_cam_height_min:
		opts->cam_height_min = strtof(v, NULL);
		break;
	case opt_cam_pitch_max:
		opts->cam_pitch_max = strdeg_to_rad(v);
		break;
	case opt_cam_pitch_min:
		opts->cam_pitch_min = strdeg_to_rad(v);
		break;
	case opt_cam_yaw:
		opts->cam_yaw = strdeg_to_rad(v);
		break;
	case opt_water:
		opts->water = str_to_bool(v);
		break;
	default:
		LOG_W("invalid option: %s", k);
		return false;
	}

	return true;
}

bool
parse_opengl_cfg(struct opengl_opts *opts)
{
	return parse_cfg_file(OPENGL_CFG, opts, parse_opengl_cfg_handler);
}
