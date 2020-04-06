#include "client/input/mouse.h"
#include "shared/util/log.h"

void
handle_mouse(int x, int y, uint64_t bstate, struct hiface *hf)
{
	L("got mouse event: %d, %d, %lx", x, y, bstate);

	hf->cursor.x = x;
	hf->cursor.y = y;
}
