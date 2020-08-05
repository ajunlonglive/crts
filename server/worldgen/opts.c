#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "server/worldgen/gen.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

#define WORLDGEN_CFG "worldgen.ini"

enum worldgen_opt {
	opt_fault_max_len,
	opt_fault_valley_chance,
	opt_fault_valley_max,
	opt_fault_valley_mod,
	opt_fault_mtn_mod,
	opt_fault_valley_min,
	opt_raindrop_max_iterations,
	opt_final_noise_octs,
	opt_height,
	opt_width,
	opt_points,
	opt_faults,
	opt_raindrops,
	opt_upscale,
	opt_seed,
	opt_radius,
	opt_fault_radius_pct_extent,
	opt_fault_max_ang,
	opt_fault_boost_decay,
	opt_erosion_rate,
	opt_deposition_rate,
	opt_raindrop_friction,
	opt_raindrop_speed,
	opt_final_noise_amp,
	opt_final_noise_freq,
	opt_final_noise_lacu,
};

static struct cfg_lookup_table keys =  {
	"fault_max_len", opt_fault_max_len,
	"fault_valley_chance", opt_fault_valley_chance,
	"fault_valley_max", opt_fault_valley_max,
	"fault_valley_mod", opt_fault_valley_mod,
	"fault_mtn_mod", opt_fault_mtn_mod,
	"fault_valley_min", opt_fault_valley_min,
	"raindrop_max_iterations", opt_raindrop_max_iterations,
	"final_noise_octs", opt_final_noise_octs,
	"height", opt_height,
	"width", opt_width,
	"points", opt_points,
	"faults", opt_faults,
	"raindrops", opt_raindrops,
	"upscale", opt_upscale,
	"seed", opt_seed,
	"radius", opt_radius,
	"fault_radius_pct_extent", opt_fault_radius_pct_extent,
	"fault_max_ang", opt_fault_max_ang,
	"fault_boost_decay", opt_fault_boost_decay,
	"erosion_rate", opt_erosion_rate,
	"deposition_rate", opt_deposition_rate,
	"raindrop_friction", opt_raindrop_friction,
	"raindrop_speed", opt_raindrop_speed,
	"final_noise_amp", opt_final_noise_amp,
	"final_noise_freq", opt_final_noise_freq,
	"final_noise_lacu", opt_final_noise_lacu,
};

static bool
parse_worldgen_cfg_handler(void *vp, const char *sec, const char *k,
	const char *v, uint32_t line)
{
	struct worldgen_opts *opts = vp;

	switch (cfg_string_lookup(k, &keys)) {
	case opt_fault_max_len:
		opts->fault_max_len = strtoul(v, NULL, 10);
		break;
	case opt_fault_valley_chance:
		opts->fault_valley_chance = strtoul(v, NULL, 10);
		break;
	case opt_fault_valley_max:
		opts->fault_valley_max = strtoul(v, NULL, 10);
		break;
	case opt_fault_valley_mod:
		opts->fault_valley_mod = strtoul(v, NULL, 10);
		break;
	case opt_fault_mtn_mod:
		opts->fault_mtn_mod = strtoul(v, NULL, 10);
		break;
	case opt_fault_valley_min:
		opts->fault_valley_min = strtoul(v, NULL, 10);
		break;
	case opt_raindrop_max_iterations:
		opts->raindrop_max_iterations = strtoul(v, NULL, 10);
		break;
	case opt_final_noise_octs:
		opts->final_noise_octs = strtoul(v, NULL, 10);
		break;
	case opt_height:
		opts->height = strtoul(v, NULL, 10);
		break;
	case opt_width:
		opts->width = strtoul(v, NULL, 10);
		break;
	case opt_points:
		opts->points = strtoul(v, NULL, 10);
		break;
	case opt_faults:
		opts->faults = strtoul(v, NULL, 10);
		break;
	case opt_raindrops:
		opts->raindrops = strtoul(v, NULL, 10);
		break;
	case opt_upscale:
		opts->upscale = strtoul(v, NULL, 10);
		break;
	case opt_seed:
		opts->seed = strtoul(v, NULL, 10);
		break;
	case opt_radius:
		opts->radius = strtof(v, NULL);
		break;
	case opt_fault_radius_pct_extent:
		opts->fault_radius_pct_extent = strtof(v, NULL);
		break;
	case opt_fault_max_ang:
		opts->fault_max_ang = strdeg_to_rad(v);
		break;
	case opt_fault_boost_decay:
		opts->fault_boost_decay = strtof(v, NULL);
		break;
	case opt_erosion_rate:
		opts->erosion_rate = strtof(v, NULL);
		break;
	case opt_deposition_rate:
		opts->deposition_rate = strtof(v, NULL);
		break;
	case opt_raindrop_friction:
		opts->raindrop_friction = strtof(v, NULL);
		break;
	case opt_raindrop_speed:
		opts->raindrop_speed = strtof(v, NULL);
		break;
	case opt_final_noise_amp:
		opts->final_noise_amp = strtof(v, NULL);
		break;
	case opt_final_noise_freq:
		opts->final_noise_freq = strtof(v, NULL);
		break;
	case opt_final_noise_lacu:
		opts->final_noise_lacu = strtof(v, NULL);
		break;
	default:
		LOG_W("invalid option: %s", k);
		return false;
	}

	return true;
}

bool
parse_worldgen_cfg(struct worldgen_opts *opts)
{
	return parse_cfg_file(WORLDGEN_CFG, opts, parse_worldgen_cfg_handler);
}
