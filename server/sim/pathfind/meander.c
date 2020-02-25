#define _XOPEN_SOURCE 500

#include <stdlib.h>

#include "server/sim/pathfind/meander.h"
#include "server/sim/terrain.h"
#include "shared/util/log.h"

void
meander(struct chunks *cnks, struct point *pos)
{
	uint8_t choice = random() % 4;
	struct point np = *pos;

	switch (choice) {
	case 0:
		np.x += 1;
		break;
	case 1:
		np.x -= 1;
		break;
	case 2:
		np.y += 1;
		break;
	case 3:
		np.y -= 1;
		break;
	}

	if (is_traversable(cnks, &np)) {
		*pos = np;
	}
}
