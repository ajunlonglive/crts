#include <string.h>

#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"

void
ent_init(struct ent *e)
{
	memset(e, 0, sizeof(struct ent));

	e->satisfaction = 100;
	e->alignment = alignment_init();
	e->idle = true;
}
