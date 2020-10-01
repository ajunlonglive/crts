#include "posix.h"

#include <ctype.h>
#include <string.h>

#include "client/input/cmdline.h"
#include "client/ui/common.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

static enum cmd_result
cmd_quit(struct cmd_ctx *_cmd, struct hiface *hf)
{
	hf->sim->run = 0;

	return cmdres_ok;
}

static enum cmd_result
cmd_clear(struct cmd_ctx *_cmd, struct hiface *hf)
{
	hf->cmdline.history.len = 0;

	return cmdres_ok;
}

static const struct cmd_table universal_cmds[] = {
	"quit", (cmdfunc)cmd_quit,
	"clear", (cmdfunc)cmd_clear
};
static const size_t universal_cmds_len =
	sizeof(universal_cmds) / sizeof(universal_cmds[0]);

static void
run_cmd(struct hiface *hf, struct cmd_ctx *cmd_ctx)
{
	cmdfunc action;

	memcpy(cmd_ctx->cmdline, hf->cmdline.cur.buf, hf->cmdline.cur.len);
	char *p = cmd_ctx->cmdline;

	while (*p) {
		while (*p && is_whitespace(*p)) {
			*p = 0;
			++p;
		}

		cmd_ctx->argv[cmd_ctx->argc++] = p;

		while (*p && !is_whitespace(*p)) {
			++p;
		}
	}

	enum cmd_result res;

	if ((action = cmd_lookup(cmd_ctx, universal_cmds, universal_cmds_len))) {
		res = action(cmd_ctx, hf);
	} else {
		res = ui_cmdline_hook(cmd_ctx, hf->ui_ctx, hf);
	}

	switch (res) {
	case cmdres_ok:
		break;
	case cmdres_not_found:
		snprintf(cmd_ctx->out, CMDLINE_BUF_LEN,
			"error: command '%s' not found", cmd_ctx->argv[0]);
		break;
	case cmdres_arg_error:
		snprintf(cmd_ctx->out, CMDLINE_BUF_LEN,
			"error: invalid arguments for '%s'", cmd_ctx->argv[0]);
		break;
	}
}

cmdfunc
cmd_lookup(const struct cmd_ctx *cmd, const struct cmd_table *tbl, size_t tbl_len)
{
	size_t i;

	for (i = 0; i < tbl_len; ++i) {
		if (strncmp(cmd->argv[0], tbl[i].cmd, strlen(tbl[i].cmd)) == 0) {
			return tbl[i].action;
		}
	}

	return NULL;
}

void
parse_cmd_input(struct hiface *hf, unsigned k)
{
	struct cmdline_buf *hbf = &hf->cmdline.cur;

	switch (k) {
	case '\b':
		if (!hbf->cursor) {
			hf->im = im_normal;
			return;
		}

		memmove(&hbf->buf[hbf->cursor - 1],
			&hbf->buf[hbf->cursor], hbf->len - hbf->cursor);

		--hbf->len;
		hbf->buf[hbf->len] = 0;

		--hbf->cursor;
		break;
	case skc_left:
		if (!hbf->cursor) {
			return;
		}

		--hbf->cursor;
		break;
	case skc_right:
		if (hbf->cursor >= hbf->len) {
			return;
		}

		++hbf->cursor;
		break;
	case skc_up:
		if (!hf->cmdline.history.cursor) {
			memcpy(&hf->cmdline.tmp, hbf, sizeof(struct cmdline_buf));
		}

		if (hf->cmdline.history.cursor < hf->cmdline.history.len) {
			++hf->cmdline.history.cursor;
		}

		memcpy(hbf->buf,
			&hf->cmdline.history.in[hf->cmdline.history.cursor - 1],
			CMDLINE_BUF_LEN);

		hbf->cursor = hbf->len = strlen(hbf->buf);
		break;
	case skc_down:
		if (!hf->cmdline.history.cursor) {
			return;
		}

		if (--hf->cmdline.history.cursor) {
			memcpy(hbf->buf,
				&hf->cmdline.history.in[hf->cmdline.history.cursor - 1],
				CMDLINE_BUF_LEN);
		} else {
			memcpy(hbf, &hf->cmdline.tmp, sizeof(struct cmdline_buf));
		}

		hbf->cursor = hbf->len = strlen(hbf->buf);
		break;
	case '\n':
		if (!hbf->len) {
			return;
		}

		struct cmd_ctx cmd_ctx = { 0 };
		run_cmd(hf, &cmd_ctx);

		memmove(&hf->cmdline.history.in[1],
			&hf->cmdline.history.in[0],
			CMDLINE_BUF_LEN * (CMDLINE_HIST_LEN - 1));
		memmove(&hf->cmdline.history.out[1],
			&hf->cmdline.history.out[0],
			CMDLINE_BUF_LEN * (CMDLINE_HIST_LEN - 1));

		memcpy(&hf->cmdline.history.in[0], hbf->buf,
			CMDLINE_BUF_LEN);
		memcpy(&hf->cmdline.history.out[0], cmd_ctx.out,
			CMDLINE_BUF_LEN);

		memset(hbf, 0, sizeof(struct cmdline_buf));

		if (hf->cmdline.history.len < CMDLINE_HIST_LEN) {
			++hf->cmdline.history.len;
		}

		hf->cmdline.history.cursor = 0;
		break;
	default:
		if (hbf->len >= HF_BUF_LEN) {
			return;
		}

		memmove(&hbf->buf[hbf->cursor + 1], &hbf->buf[hbf->cursor],
			hbf->len - hbf->cursor);

		hbf->buf[hbf->cursor] = k;
		++hbf->len;
		++hbf->cursor;
	}
}
