#include "posix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "launcher/save_settings.h"
#include "shared/platform/common/dirs.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

#define PATH_MAX 2048

const char *
settings_file_path(void)
{
	static char buf[PATH_MAX];

	snprintf(buf, PATH_MAX, "%s/config.ini", platform_config_dir());
	return buf;
}

bool
save_settings(struct client_opts *opts)
{
	const char *path = settings_file_path();
	FILE *f;
	if (!(f = fopen(path, "wb"))) {
		return false;
	}

	fprintf(f, "[sound]\n");
	fprintf(f, "device = %d\n", opts->sound.device);
	fprintf(f, "master = %f\n", opts->sound.master);
	fprintf(f, "music = %f\n", opts->sound.music);
	fprintf(f, "sfx = %f\n", opts->sound.sfx);

	fprintf(f, "\n");

	fprintf(f, "[ui]\n");
	fprintf(f, "scale = %f\n", opts->ui_cfg.ui_scale);
	fprintf(f, "mouse_sens = %f\n", opts->ui_cfg.mouse_sensitivity);

	fclose(f);
	return true;
}

static bool
parse_settings_handler(void *_ctx, char *err, const char *sec, const char *k, const char *v, uint32_t line)
{
	struct client_opts *ctx = _ctx;

	enum val_type {
		type_uint,
		type_float,
	};

	struct keygroup {
		const char *name;
		enum val_type type;
		uint32_t off;
	};

	const struct keygroup sound_keys[] = {
		{ "device", type_uint, offsetof(struct client_opts, sound.device) },
		{ "master", type_float, offsetof(struct client_opts, sound.master) },
		{ "music", type_float, offsetof(struct client_opts, sound.music) },
		{ "sfx", type_float, offsetof(struct client_opts, sound.sfx) },
		0
	}, ui_keys[] = {
		{ "scale", type_float, offsetof(struct client_opts, ui_cfg.ui_scale) },
		{ "mouse_sens", type_float, offsetof(struct client_opts, ui_cfg.mouse_sensitivity) },
		0
	}, *group;

	if (strcmp(sec, "sound") == 0) {
		group = sound_keys;
	} else if (strcmp(sec, "ui") == 0) {
		group = ui_keys;
	} else {
		LOG_W(log_misc, "invalid section '%s'", sec);
		return false;
	}

	uint32_t i;
	for (i = 0; group[i].name; ++i) {
		if (strcmp(k, group[i].name) != 0) {
			continue;
		}

		void *val_dest = ((uint8_t *)ctx + group[i].off);

		switch (group[i].type) {
		case type_uint: {
			char *endptr = NULL;
			long lval = strtol(v, &endptr, 10);
			if (*endptr) {
				LOG_W(log_misc, "unable to parse integer");
				return false;
			} else if (lval < 0 || lval > (long)UINT32_MAX) {
				LOG_W(log_misc, "integer outside of range 0-%u", UINT32_MAX);
				return false;
			}

			uint32_t val = lval;
			memcpy(val_dest, &val, sizeof(uint32_t));
			break;
		}
		case type_float: {
			char *endptr = NULL;
			float fval = strtof(v, &endptr);
			if (*endptr) {
				LOG_W(log_misc, "unable to parse float");
				return false;
			}

			memcpy(val_dest, &fval, sizeof(float));
			break;
		}
		}
	}

	return true;
}

bool
load_settings(struct client_opts *opts)
{
	if (!parse_cfg_file(settings_file_path(), opts, parse_settings_handler)) {
		LOG_W(log_misc, "failed to parse settings");
		return false;
	}

	return true;
}
