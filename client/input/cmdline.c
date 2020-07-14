#include "posix.h"

#include <string.h>

#include "client/input/cmdline.h"
#include "shared/util/log.h"

static void
run_cmd(struct hiface *hf)
{
	struct hiface_buf *hbf = &hf->cmdline;

	L("got cmd: '%s'", hbf->buf);
}

void
parse_cmd_input(struct hiface *hf, unsigned k)
{
	struct hiface_buf *hbf = &hf->cmdline;

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
		if (!hf->cmdline_history.cursor) {
			memcpy(&hf->cmdline_history.tmp, hbf, sizeof(struct hiface_buf));
		}

		if (hf->cmdline_history.cursor < hf->cmdline_history.len) {
			++hf->cmdline_history.cursor;
		}

		memcpy(hbf,
			&hf->cmdline_history.items[hf->cmdline_history.cursor - 1],
			sizeof(struct hiface_buf));
		break;
	case skc_down:
		if (!hf->cmdline_history.cursor) {
			return;
		}

		if (--hf->cmdline_history.cursor) {
			memcpy(hbf,
				&hf->cmdline_history.items[hf->cmdline_history.cursor - 1],
				sizeof(struct hiface_buf));
		} else {
			memcpy(hbf, &hf->cmdline_history.tmp, sizeof(struct hiface_buf));
		}
		break;
	case '\n':
		if (!hbf->len) {
			return;
		}

		run_cmd(hf);

		memmove(&hf->cmdline_history.items[1],
			&hf->cmdline_history.items[0],
			sizeof(struct hiface_buf) * (HF_HIST_LEN - 1));

		memcpy(&hf->cmdline_history.items[0], hbf,
			sizeof(struct hiface_buf));

		memset(hbf, 0, sizeof(struct hiface_buf));

		if (hf->cmdline_history.len < HF_HIST_LEN) {
			++hf->cmdline_history.len;
		}

		hf->cmdline_history.cursor = 0;
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

