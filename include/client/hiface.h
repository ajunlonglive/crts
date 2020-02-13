#ifndef __DISPLAY_H
#define __DISPLAY_H
#include "sim.h"
#include "input/modes.h"
#include "shared/types/geom.h"

struct hiface {
	struct simulation *sim;

	struct point cursor;
	struct point view;
	enum input_mode im;
	struct keymap *km;
};

struct hiface *hiface_init(struct simulation *sim);
#endif
