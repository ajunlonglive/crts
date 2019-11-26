#ifndef __STATE_H
#define __STATE_H
#include <pthread.h>
#include "geom.h"

struct state {
	struct {
		struct win *root;
		struct win *_info;
		struct win *infol;
		struct win *infor;
		struct win *world;
	} wins;

	struct {
		pthread_t receive;
		pthread_t respond;
		pthread_t update;
	} threads;

	struct point view;

	struct cxinfo *cx;

	struct world *w;

	int run;
};

void state_init(struct state *s);
#endif
