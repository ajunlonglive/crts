#include "posix.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "client/input/cmdline.h"
#include "client/input/handler.h"
#include "client/input/keymap.h"
/* #include "client/net.h" */
#include "client/ui/common.h"
#include "shared/serialize/to_disk.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

static enum cmd_result
cmd_quit(struct cmd_ctx *_cmd, struct client *cli)
{
	cli->run = false;

	return cmdres_ok;
}

static enum cmd_result
cmd_clear(struct cmd_ctx *_cmd, struct client *cli)
{
	cli->cmdline.history.len = 0;

	return cmdres_dont_keep_hist;
}

static enum cmd_result
cmd_connect(struct cmd_ctx *cmd, struct client *cli)
{
	if (cmd->argc < 2) {
		return cmdres_arg_error;
	}

	snprintf(cmd->out, CMDLINE_BUF_LEN, "connecting to %s", cmd->argv[1]);

	hdarr_clear(&cli->world->chunks.hd);
	hdarr_clear(&cli->world->ents);


	/* cx_pool_clear(&cli->nx->cxs); */
	/* set_server_address(cmd->argv[1]); */

	return cmdres_ok;
}

static enum cmd_result
cmd_load(struct cmd_ctx *cmd, struct client *cli)
{
	FILE *f;

	if (cmd->argc < 2) {
		return cmdres_arg_error;
	} else if (cli->msgr) {
		/* TODO: this should be possible, need a way to toggle
		 * connectivity */
		snprintf(cmd->out, CMDLINE_BUF_LEN,
			"this command may only be used offline");
		return cmdres_cmd_error;
	} else if (!(f = fopen(cmd->argv[1], "rb"))) {
		snprintf(cmd->out, CMDLINE_BUF_LEN, "unable to read '%s': %s",
			cmd->argv[1], strerror(errno));
		return cmdres_cmd_error;
	}

	hdarr_clear(&cli->world->chunks.hd);

	read_chunks(f, &cli->world->chunks);
	fclose(f);

	cli->changed.chunks = true;

	snprintf(cmd->out, CMDLINE_BUF_LEN, "loaded %s", cmd->argv[1]);

	return cmdres_ok;
}


static enum cmd_result
cmd_goto(struct cmd_ctx *cmd, struct client *cli)
{
	if (cmd->argc < 3) {
		return cmdres_arg_error;
	}

	long x, y;

	x = strtol(cmd->argv[1], NULL, 10);
	y = strtol(cmd->argv[2], NULL, 10);

	cli->view.x = x;
	cli->view.y = y;
	cli->cursor.x = 0;
	cli->cursor.y = 0;

	trigger_cmd(kc_center_cursor, cli);

	snprintf(cmd->out, CMDLINE_BUF_LEN,
		"centering view on (%ld, %ld)", x, y);

	return cmdres_ok;
}

static enum cmd_result
run_key_command(struct cmd_ctx *cmd, struct client *cli, enum key_command kc)
{
	if (cmd->argc > 2) {
		return cmdres_arg_error;
	} else if (cmd->argc > 1) {
		cli->num_override.override = true;
		int32_t val;
		if ((val = cfg_string_lookup(cmd->argv[1],
			&cmd_string_lookup_tables[cslt_constants])) != -1) {
			cli->num_override.val = val;
		} else {
			cli->num_override.val = strtol(cmd->argv[1], NULL, 10);
		}
	}

	trigger_cmd(kc, cli);

	return cmdres_ok;
}

static const struct cmd_table universal_cmds[] = {
	"quit", (cmdfunc)cmd_quit,
	"clear", (cmdfunc)cmd_clear,
	"connect", (cmdfunc)cmd_connect,
	"load", (cmdfunc)cmd_load,
	"goto", (cmdfunc)cmd_goto,
};

static const size_t universal_cmds_len =
	sizeof(universal_cmds) / sizeof(universal_cmds[0]);

static void
run_cmd(struct client *cli, struct cmd_ctx *cmd_ctx)
{
	cmdfunc action;

	memcpy(cmd_ctx->cmdline, cli->cmdline.cur.buf, cli->cmdline.cur.len);
	char *p = cmd_ctx->cmdline;
	static char buf[256] = { 0 };
	uint32_t len;

	while (*p) {
		while (*p && is_whitespace(*p)) {
			*p = 0;
			++p;
		}

		if (!*p) {
			break;
		}

		cmd_ctx->argv[cmd_ctx->argc++] = p;

		while (*p && !is_whitespace(*p)) {
			++p;
		}
	}

	if (*cmd_ctx->argv[0] == '!') {
		int32_t kc;

		if ((kc = cfg_string_lookup(&cmd_ctx->argv[0][1],
			&cmd_string_lookup_tables[cslt_commands])) == -1) {
			cmd_ctx->res = cmdres_not_found;
		} else {
			cmd_ctx->res = run_key_command(cmd_ctx, cli, kc);
		}
	} else if ((action = cmd_lookup(cmd_ctx, universal_cmds, universal_cmds_len))) {
		cmd_ctx->res = action(cmd_ctx, cli);
	} else {
		cmd_ctx->res = ui_cmdline_hook(cmd_ctx, cli);
	}

	switch (cmd_ctx->res) {
	case cmdres_ok:
	case cmdres_dont_keep_hist:
		break;
	case cmdres_not_found:
		snprintf(cmd_ctx->out, CMDLINE_BUF_LEN,
			"error: command '%s' not found", cmd_ctx->argv[0]);
		break;
	case cmdres_arg_error:
		snprintf(cmd_ctx->out, CMDLINE_BUF_LEN,
			"error: invalid arguments for '%s'", cmd_ctx->argv[0]);
		break;
	case cmdres_cmd_error:
		len = snprintf(buf, CMDLINE_BUF_LEN, "error: %s", cmd_ctx->out);
		strncpy(cmd_ctx->out, buf, len);
		break;
	}
}

cmdfunc
cmd_lookup(const struct cmd_ctx *cmd, const struct cmd_table *tbl, size_t tbl_len)
{
	size_t i, len = strlen(cmd->argv[0]);

	for (i = 0; i < tbl_len; ++i) {
		size_t cmdlen = strlen(tbl[i].cmd);
		if (len == cmdlen && strncmp(cmd->argv[0], tbl[i].cmd, len) == 0) {
			return tbl[i].action;
		}
	}

	return NULL;
}

void
parse_cmd_input(struct client *cli, unsigned k)
{
	struct cmdline_buf *hbf = &cli->cmdline.cur;

	switch (k) {
	case '\b':
		if (!hbf->cursor) {
			goto exit_cmdline;
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
		if (!cli->cmdline.history.cursor) {
			memcpy(&cli->cmdline.tmp, hbf, sizeof(struct cmdline_buf));
		}

		if (cli->cmdline.history.cursor < cli->cmdline.history.len) {
			++cli->cmdline.history.cursor;
		}

		memcpy(hbf->buf,
			&cli->cmdline.history.in[cli->cmdline.history.cursor - 1],
			CMDLINE_BUF_LEN);

		hbf->cursor = hbf->len = strlen(hbf->buf);
		break;
	case skc_down:
		if (!cli->cmdline.history.cursor) {
			return;
		}

		if (--cli->cmdline.history.cursor) {
			memcpy(hbf->buf,
				&cli->cmdline.history.in[cli->cmdline.history.cursor - 1],
				CMDLINE_BUF_LEN);
		} else {
			memcpy(hbf, &cli->cmdline.tmp, sizeof(struct cmdline_buf));
		}

		hbf->cursor = hbf->len = strlen(hbf->buf);
		break;
	case '\n':
		if (!hbf->cursor) {
			goto exit_cmdline;
		}

		struct cmd_ctx cmd_ctx = { 0 };

		run_cmd(cli, &cmd_ctx);

		if (cmd_ctx.res != cmdres_dont_keep_hist) {
			memmove(&cli->cmdline.history.in[1],
				&cli->cmdline.history.in[0],
				CMDLINE_BUF_LEN * (CMDLINE_HIST_LEN - 1));
			memmove(&cli->cmdline.history.out[1],
				&cli->cmdline.history.out[0],
				CMDLINE_BUF_LEN * (CMDLINE_HIST_LEN - 1));

			memcpy(&cli->cmdline.history.in[0], hbf->buf,
				CMDLINE_BUF_LEN);
			memcpy(&cli->cmdline.history.out[0], cmd_ctx.out,
				CMDLINE_BUF_LEN);

			if (cli->cmdline.history.len < CMDLINE_HIST_LEN) {
				++cli->cmdline.history.len;
			}
		}

		memset(hbf, 0, sizeof(struct cmdline_buf));

		cli->cmdline.history.cursor = 0;
		break;
	default:
		if (hbf->len >= INPUT_BUF_LEN) {
			return;
		}

		memmove(&hbf->buf[hbf->cursor + 1], &hbf->buf[hbf->cursor],
			hbf->len - hbf->cursor);

		hbf->buf[hbf->cursor] = k;
		++hbf->len;
		++hbf->cursor;
	}

	return;

exit_cmdline:
	cli->im = im_normal;
}

void
run_cmd_string(struct client *cli, const char *cmds)
{
	const char *p, *start = cmds;
	uint32_t len = 0;

	for (p = cmds;; ++p) {
		if (len && (*p == ';' || *p == 0)) {
			memcpy(cli->cmdline.cur.buf, start, len);
			cli->cmdline.cur.len = len;

			struct cmd_ctx cmd_ctx = { 0 };
			run_cmd(cli, &cmd_ctx);

			switch (cmd_ctx.res) {
			case cmdres_dont_keep_hist:
			case cmdres_ok:
				LOG_I(log_misc, "%s:%s", cmd_ctx.cmdline, cmd_ctx.out);
				break;
			case cmdres_not_found:
			case cmdres_arg_error:
			case cmdres_cmd_error:
				LOG_W(log_misc, "%s:%s", cmd_ctx.cmdline, cmd_ctx.out);
				break;
			}

			memset(cli->cmdline.cur.buf, 0, CMDLINE_BUF_LEN);
			cli->cmdline.cur.len = 0;

			if (*p) {
				if (!*(start = ++p)) {
					break;
				}
				len = 1;
			} else {
				break;
			}
		} else {
			++len;
		}
	}
}
