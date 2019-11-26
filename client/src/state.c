#include <string.h>
#include "state.h"

void state_init(struct state *s)
{
	memset(s, 0, sizeof(struct state));

	s->run = 1;
	s->mode = view_mode_normal;
}
