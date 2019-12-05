#ifndef __DISPLAY_H
#define __DISPLAY_H
#include "sim.h"
#include "types/geom.h"

struct display {
	struct simulation *sim;

	struct point cursor;
	struct point view;
};

void display(struct simulation *sim);
#endif
