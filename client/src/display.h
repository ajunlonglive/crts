#ifndef __DISPLAY_H
#define __DISPLAY_H
#include "sim.h"
#include "input/modes.h"
#include "types/geom.h"

struct display {
	struct simulation *sim;

	struct point cursor;
	struct point view;
	enum input_mode im;
};

void display(struct simulation *sim);
#endif
