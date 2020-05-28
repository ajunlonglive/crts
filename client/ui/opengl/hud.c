#include "posix.h"

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/hud.h"
#include "client/ui/opengl/text.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"

void
render_hud(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	const char *act_tgt_nme;

	text_setup_render();

	gl_printf(0, 1, "redraws: (+%6d) | cmd: %5.5s%5.5s",
		hf->redrew_world, hf->num.buf, hf->cmd.buf);

	gl_printf(0, 0, "view: (%4d, %4d) | cursor: (%4d, %4d)",
		hf->view.x, hf->view.y, hf->cursor.x + hf->view.x,
		hf->cursor.y + hf->view.y);

	switch (hf->next_act.type) {
	case at_harvest:
		act_tgt_nme = gcfg.tiles[hf->next_act.tgt].name;
		break;
	case at_build:
		act_tgt_nme = blueprints[hf->next_act.tgt].name;
		break;
	default:
		act_tgt_nme = "";
		break;
	}

	gl_printf(0, 2, "act: %s%c %s, %x",
		gcfg.actions[hf->next_act.type].name,
		*act_tgt_nme ? ',' : ' ', act_tgt_nme,
		hf->next_act.flags);
}
