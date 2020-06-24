#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render/hud.h"
#include "client/ui/opengl/render/text.h"
#include "shared/constants/globals.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

#define MENU_GLUE 1.5

struct menu {
	uint8_t indices[64];
	char *desc[64];
	char *trigger[64];
	uint32_t len;
	uint32_t max_trigger_len;
	uint32_t pref_len;
	int32_t seli;
	struct keymap *selp;
};

vec4 menu_clr    = { 1, 1,   1, 0.5 };
vec4 sel_clr     = { 1, 1,   0, 0.9 };
vec4 typed_clr   = { 0, 1,   0, 0.9 };
vec4 to_type_clr = { 0, 0.5, 0, 0.9 };

static struct menu completions;

#define SENS 0.05
#define SCALE 1
#define MENU_R 15.0
#define SPIN (PI / 4)
#define BUMP

static uint8_t
write_menu(float x, float y, struct hiface_buf *cmd, struct menu *m, struct hiface *hf)
{

	size_t len = strlen(cmd->buf);
	size_t numlen = strlen(hf->num.buf);

	uint16_t i = 0, j;
	int16_t sel_i = -1;
	float yp, shift;

	gl_printf(x, y, "%s mode", input_mode_names[hf->im]);

	for (j = 0; j < m->len; ++j) {
		i = completions.indices[j];
		yp = y - (j + 1) * 1.1f;
		shift = completions.max_trigger_len + numlen + 1;

		if (numlen) {
			gl_write_string(x, yp, SCALE, typed_clr, hf->num.buf);
		}

		if (completions.seli == -1) {
			gl_write_string(x + numlen, yp, SCALE, typed_clr, cmd->buf);

			gl_write_string(x + len + numlen, yp, SCALE, to_type_clr, &m->trigger[i][len]);
		} else if (completions.seli == i) {
			gl_write_string(x + numlen, yp, SCALE, sel_clr, m->trigger[i]);
		} else {
			gl_write_string(x + numlen, yp, SCALE, to_type_clr, m->trigger[i]);
		}

		gl_write_string(x + shift, yp, SCALE,
			completions.seli == i ? sel_clr : menu_clr,
			m->desc[i]);
	}

	return sel_i;
}

static int
compare_completion_menu_item(const void *_a, const void *_b)
{
	uint8_t a = *(uint8_t *)_a, b = *(uint8_t *)_b;

	return strcmp(completions.desc[a], completions.desc[b]);
}

static void
add_completion_menu_item(void *_ctx, struct keymap *km)
{
	//struct opengl_ui_ctx *ctx = _ctx;

	if (*km->desc == '\0') {
		return;
	}

	if (km->trigger_len > completions.max_trigger_len) {
		completions.max_trigger_len = km->trigger_len;
	}

	if (km == completions.selp) {
		completions.seli = completions.len;
	}

	completions.indices[completions.len] = completions.len;

	completions.trigger[completions.len] = km->trigger;

	completions.desc[completions.len] = km->desc;

	++completions.len;
}

static void
render_completions(float x, float y, struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	static bool regen_menu = true;
	struct hiface_buf cmd = hf->cmd;

	if (regen_menu || hf->input_changed) {
		regen_menu = false;
		completions.seli = -1;
		completions.selp = NULL;
		completions.len = 0;
		completions.pref_len = 0;
		completions.max_trigger_len = 0;

		if (cmd.len == 0 && ctx->last_key) {
			struct keymap *tmp = ctx->ckm;
			struct im;
			ctx->ckm = ctx->okm;

			if ((completions.selp = &ctx->ckm->map[(uint8_t)ctx->last_key])->cmd) {
				hifb_append_char(&cmd, ctx->last_key);
			} else {
				completions.selp = NULL;
				ctx->ckm = tmp;
			}

			ctx->last_key = 0;
			regen_menu = true;
		}

		enum input_mode im = hf->im;
		hf->im = ctx->oim;
		describe_completions(hf, ctx->ckm, ctx, add_completion_menu_item);
		hf->im = im;

		qsort(completions.indices, completions.len, sizeof(uint8_t),
			compare_completion_menu_item);
	}

	write_menu(x, y, &cmd, &completions, hf);
}

void
render_hud(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	const char *act_tgt_nme;
	float sx, sy;
	/*
	   x and y at in game cursor position
	   x = (float)hf->cursor.x / ctx->ref.width * ctx->width;
	   y = (float)hf->cursor.y / ctx->ref.height * ctx->height;
	 */
	/*
	   x = ctx->width * 0.5;
	   y = ctx->height * 0.5;
	 */
	screen_coords_to_text_coords(2, -3, &sx, &sy);

	text_setup_render(ctx);

	gl_printf(0, 1, "cmd: %5.5s%5.5s | im: %s",
		hf->num.buf, hf->cmd.buf, input_mode_names[hf->im]);

	gl_printf(0, 0, "view: (%4d, %4d) | cursor: (%4d, %4d) | cx: %d",
		hf->view.x, hf->view.y, hf->cursor.x + hf->view.x,
		hf->cursor.y + hf->view.y,
		hdarr_len(hf->nx->cxs.cxs) > 0
			? ((struct connection *)darr_get(hdarr_darr(hf->nx->cxs.cxs), 0))->stale
			: UINT32_MAX
		);

	switch (hf->next_act.type) {
	case at_build:
		act_tgt_nme = gcfg.tiles[hf->next_act.tgt].name;
		break;
	default:
		act_tgt_nme = NULL;
		break;
	}

	gl_printf(0, 2, "act: 0x%x %s%c %s",
		hf->next_act.flags,
		gcfg.actions[hf->next_act.type].name,
		act_tgt_nme ? ',' : ' ',
		act_tgt_nme);

	gl_printf(0, 3, "mouse: 0x%x", ctx->mouse.buttons);

	render_completions(sx, sy, ctx, hf);
}

/* must be called AFTER render_hud */
void
render_debug_hud(struct opengl_ui_ctx *ctx)
{
	float sx, sy;
	screen_coords_to_text_coords(0, -1, &sx, &sy);
	gl_printf(sx, sy, "t: %.2fms (%.1f fps) | s: %.1f%%, r: %.1f%%",
		ctx->prof.ftime * 1000,
		1 / ctx->prof.ftime,
		100 * ctx->prof.setup / ctx->prof.ftime,
		100 * ctx->prof.render / ctx->prof.ftime);

	screen_coords_to_text_coords(0, -2, &sx, &sy);
	gl_printf(sx, sy, "cam: %.2f,%.2f,%.2f p: %.1f y: %.1f",
		cam.pos[0],
		cam.pos[1],
		cam.pos[2],
		cam.pitch  * (180.0f / PI),
		cam.yaw * (180.0f / PI));
}
