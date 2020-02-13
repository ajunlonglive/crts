#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"

void
ent_init(struct ent *e)
{
	e->id = 0;
	e->pos.x = 0;
	e->pos.y = 0;
	e->c = '?';

	e->satisfaction = 100;
	e->alignment = alignment_init();
	e->age = 0;
	e->task = 0;
	e->idle = 1;
}

