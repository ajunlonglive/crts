#include "client/input/action_handler.h"
#include "client/input/mouse.h"
#include "shared/util/log.h"

void
handle_mouse(int x, int y, uint64_t bstate, struct hiface *hf)
{
	struct point p = { x, y }, q;

	hf->cursor = p;

	if (bstate & ms_b1_press) {
		hf->mouse.drag = true;
		hf->mouse.drag_start = p;
		hf->mouse.drag_start_view = hf->view;
	}
	if (bstate & ms_b1_release) {
		hf->mouse.drag = false;
	}

	if (bstate & ms_b3_press) {
		exec_action(hf);
	}

	if (hf->mouse.drag) {
		q = point_sub(&p, &hf->mouse.drag_start);
		hf->view = point_sub(&hf->mouse.drag_start_view, &q);
	}

	hf->next_act_changed = true;
}
