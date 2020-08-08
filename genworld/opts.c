#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "genworld/gen.h"
#include "genworld/opts.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

#define WORLDGEN_CFG "worldgen.ini"

enum worldgen_opt {
	opt_deposition_rate,
	opt_erosion_rate,
	opt_fault_boost_decay,
	opt_fault_max_ang,
	opt_fault_max_len,
	opt_fault_mtn_mod,
	opt_fault_radius_pct_extent,
	opt_fault_valley_chance,
	opt_fault_valley_max,
	opt_fault_valley_min,
	opt_fault_valley_mod,
	opt_faults,
	opt_final_noise_amp,
	opt_final_noise_freq,
	opt_final_noise_lacu,
	opt_final_noise_octs,
	opt_height,
	opt_points,
	opt_radius,
	opt_raindrop_friction,
	opt_raindrop_max_iterations,
	opt_raindrop_speed,
	opt_raindrops,
	opt_seed,
	opt_upscale,
	opt_width,
	opt_count,
};

static const struct cfg_lookup_table keys =  {
	"deposition_rate", opt_deposition_rate,
	"erosion_rate", opt_erosion_rate,
	"fault_boost_decay", opt_fault_boost_decay,
	"fault_max_ang", opt_fault_max_ang,
	"fault_max_len", opt_fault_max_len,
	"fault_mtn_mod", opt_fault_mtn_mod,
	"fault_radius_pct_extent", opt_fault_radius_pct_extent,
	"fault_valley_chance", opt_fault_valley_chance,
	"fault_valley_max", opt_fault_valley_max,
	"fault_valley_min", opt_fault_valley_min,
	"fault_valley_mod", opt_fault_valley_mod,
	"faults", opt_faults,
	"final_noise_amp", opt_final_noise_amp,
	"final_noise_freq", opt_final_noise_freq,
	"final_noise_lacu", opt_final_noise_lacu,
	"final_noise_octs", opt_final_noise_octs,
	"height", opt_height,
	"points", opt_points,
	"radius", opt_radius,
	"raindrop_friction", opt_raindrop_friction,
	"raindrop_max_iterations", opt_raindrop_max_iterations,
	"raindrop_speed", opt_raindrop_speed,
	"raindrops", opt_raindrops,
	"seed", opt_seed,
	"upscale", opt_upscale,
	"width", opt_width,
};

static const char *descs[] = {
	[opt_fault_max_len] = "the max fault lengh",
	[opt_fault_valley_chance] = "ratio of faults that are valleys",
	[opt_fault_valley_max] = "the max height of a valley",
	[opt_fault_valley_mod] = "randomly valley depth modifier",
	[opt_fault_mtn_mod] = "random mountain depth modifier",
	[opt_fault_valley_min] = "the minimum height of a valley",
	[opt_raindrop_max_iterations] = "the amount of steps to take when simulating a raindrop",
	[opt_final_noise_octs] = "how many octaves of perlin noise to use",
	[opt_height] = "the output height",
	[opt_width] = "the output width",
	[opt_points] = "the number of initial seed points to use",
	[opt_faults] = "the number of faults to generate",
	[opt_raindrops] = "the number of raindrops to simulate",
	[opt_upscale] = "upscale final output by this multiplier",
	[opt_seed] = "the seed to use",
	[opt_radius] = "the radius to scatter points in",
	[opt_fault_radius_pct_extent] = "???",
	[opt_fault_max_ang] = "the maximum angle allowed when tracing fault lines",
	[opt_fault_boost_decay] = "the steepness of valleys and mountains ",
	[opt_erosion_rate] = "the erosion rate of raindrops",
	[opt_deposition_rate] = "the deposition rate of raindrops",
	[opt_raindrop_friction] = "raindrop friction",
	[opt_raindrop_speed] = "raindrop speed",
	[opt_final_noise_amp] = "amplitude of perlin noise",
	[opt_final_noise_freq] = "frequency of perlin noise",
	[opt_final_noise_lacu] = "lacu",
};

static bool
parse_option(struct worldgen_opts *opts, const char *k, const char *v)
{
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

static void
print_usage(void)
{
	uint32_t i;

	printf("genworld - generate worlds for crts\n"
		"usage: genworld [OPTIONS]\n"
		"\n"
		"OPTIONS:\n"
		"-a <path[:path[:path]]> - set asset path\n"
		"-o opt1=val1[,opt2=val2[,...]]\n"
		"                        - set world generation options\n"
		"-i FILE                 - read world generation options from file\n"
		"-v <lvl>                - set verbosity\n"
		"-l <file>               - log to <file>\n"
		"-h                      - show this message\n"
		"WORLD GENERATION OPTIONS:\n"
		);

	for (i = 0; i < opt_count; ++i) {
		printf("%-23.23s - %s\n", keys.e[i].str, descs[keys.e[i].t]);
	}
}

static bool
parse_ini_cfg_handler(void *vp, const char *sec, const char *k,
	const char *v, uint32_t line)
{
	struct worldgen_opts *opts = vp;

	return parse_option(opts, k, v);
}

static bool
parse_optstring(char *s, struct worldgen_opts *opts)
{
	char *osp = s, *k = s, *v = NULL;

	uint32_t i;
	size_t len = strlen(s);
	char os[len + 2];
	memcpy(os, s, len);
	os[len] = os[len + 1] = 0;
	uint8_t set = 1;

	while (*s != 0) {
		switch (*s) {
		case ' ':
			continue;
		case ',':
			if (!v) {
				LOG_W("unexpected ','");
				goto parse_err;
			}

			*s = 0;

			if (!parse_option(opts, k, v)) {
				goto parse_err;
			}

			k = v = NULL;
			set = 1;
			break;
		case '=':
			if (v) {
				LOG_W("unexpected '='");
				goto parse_err;
			} else if (set) {
				LOG_W("missing key");
				goto parse_err;
			}

			*s = 0;
			set = 2;
			break;
		default:
			if (set == 1) {
				k = s;
				set = 0;
			} else if (set == 2) {
				v = s;
				set = 0;
			}
		}

		++s;
	}

	if (k && *k) {
		if (v && *v) {
			if (!parse_option(opts, k, v)) {
				goto parse_err;
			}
		} else {
			LOG_W("expecting a value");
			goto parse_err;
		}
	}

	return true;

parse_err:
	fprintf(logfile, "%s\n", os);

	for (i = 0; i < len; ++osp, ++i) {
		os[i] = osp == s ? '^' : ' ';
	}

	if (osp == s) {
		os[i] = '^';
	}

	fprintf(logfile, "%s\n", os);

	return false;
}

void
parse_cmdline_opts(int32_t argc, char *const *argv, struct genworld_opts *opts)
{
	signed char opt;

	while ((opt = getopt(argc, argv, "a:f:hil:o:v:")) != -1) {
		switch (opt) {
		case 'a':
			asset_path_init(optarg);
			break;
		case 'f':
			parse_cfg_file(optarg, &opts->opts, parse_ini_cfg_handler);
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			opts->interactive = true;
			break;
		case 'l':
			set_log_file(optarg);
			break;
		case 'o':
			parse_optstring(optarg, &opts->opts);
			break;
		case 'v':
			set_log_lvl(optarg);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
}

