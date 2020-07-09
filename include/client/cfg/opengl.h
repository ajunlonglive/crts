#ifndef CLIENT_CFG_OPENGL_H
#define CLIENT_CFG_OPENGL_H

#include <stdbool.h>
#include <stdint.h>

struct opengl_opts {
	bool shadows;
	uint32_t shadow_map_res;
	float font_scale;
	float cam_height_max, cam_height_min, cam_pitch_min, cam_pitch_max,
	      cam_pitch, cam_yaw;

	bool water;
};

bool parse_opengl_cfg(struct opengl_opts *opts);
#endif
