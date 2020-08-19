#ifndef TERRAGEN_OPTS_H
#define TERRAGEN_OPTS_H

#include <stdint.h>

enum terragen_opt {
	tg_seed,
	tg_radius,
	tg_dim,
	tg_points,

	tg_mountains,
	tg_valleys,
	tg_fault_radius,
	tg_fault_curve,
	tg_height_mod,

	tg_erosion_cycles,

	tg_noise,

	tg_upscale,

	tg_opt_count
};

enum tg_dtype { dt_float, dt_int };

struct terragen_opt_data {
	char *name;
	enum tg_dtype t;
};

extern const struct terragen_opt_data terragen_opt_info[tg_opt_count];

typedef union { float f; uint32_t u; } terragen_opts[tg_opt_count];

void tg_parse_optstring(char *s, terragen_opts opts);
void tg_parse_optfile(const char *f, terragen_opts opts);
#endif
