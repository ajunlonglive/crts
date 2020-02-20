#include <string.h>

#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

void
ent_init(struct ent *e)
{
	memset(e, 0, sizeof(struct ent));

#ifdef CRTS_SERVER
	e->alignment = alignment_init();
	e->satisfaction = 100;
	e->idle = true;
#endif
}
