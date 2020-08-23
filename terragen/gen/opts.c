#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "shared/util/text.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/opts.h"

const struct terragen_opt_data terragen_opt_info[tg_opt_count] = {
	[tg_seed] = { "seed", dt_int },
	[tg_radius] = { "radius", dt_float },
	[tg_dim] = { "dim", dt_int },
	[tg_points] = { "points", dt_int },

	[tg_mountains] = { "mountains", dt_int },
	[tg_valleys] = { "valleys", dt_int },
	[tg_fault_radius] = { "fault_radius", dt_float },
	[tg_fault_curve] = { "fault_curve", dt_float },
	[tg_height_mod] = { "height_mod", dt_float },

	[tg_erosion_cycles] = { "erosion_cycles", dt_int },

	[tg_noise] = { "noise", dt_float },

	[tg_upscale] = { "upscale", dt_int },
};

static enum terragen_opt
lookup_str_opt(const char *str)
{
	uint32_t i;
	for (i = 0; i < tg_opt_count; ++i) {
		if (strcmp(terragen_opt_info[i].name, str) == 0) {
			return i;
		}
	}

	return tg_opt_count;
}

static bool
parse_option(terragen_opts opts, const char *k, const char *v)
{
	enum terragen_opt opt;

	if ((opt = lookup_str_opt(k)) < tg_opt_count) {
		switch (terragen_opt_info[opt].t) {
		case dt_float:
			opts[opt].f = strtof(v, NULL);
			break;
		case dt_int:
			opts[opt].u = strtoul(v, NULL, 10);
			break;
		case dt_none:
			assert(false);
			break;
		}
	} else {
		LOG_W("invalid option: %s", k);
		return false;
	}

	return true;
}

static bool
parse_ini_cfg_handler(void *vp, const char *sec, const char *k,
	const char *v, uint32_t line)
{
	return parse_option(vp, k, v);
}

static bool
tg_parse_optstring_cb(void *ctx, const char *k, const char *v)
{
	return parse_option(ctx, k, v);
}

void
tg_parse_optstring(char *s, terragen_opts opts)
{
	parse_optstring(s, opts, tg_parse_optstring_cb);
}

void
tg_parse_optfile(const char *f, terragen_opts opts)
{
	parse_cfg_file(f, opts, parse_ini_cfg_handler);
}