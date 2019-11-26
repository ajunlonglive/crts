#ifndef __STATE_H
#define __STATE_H
#include <pthread.h>
#include "geom.h"

enum view_mode {
	view_mode_normal,
	view_mode_select,
};

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
	enum view_mode mode;
	struct point cursor;

	struct cxinfo *cx;

	struct world *w;

	int run;
};

void state_init(struct state *s);
#endif
