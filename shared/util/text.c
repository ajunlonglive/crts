#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "shared/util/text.h"

bool
is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void
each_line(struct file_data *fd, void *ctx, each_line_callback cb)
{
	char *line, *b;

	line = (char *)fd->data;

	while ((b = strchr(line, '\n'))) {
		*b = '\0';

		if (cb(ctx, line, b - line) != ir_cont) {
			break;
		}

		line = b + 1;

		if ((size_t)((uint8_t *)line - fd->data) >= fd->len) {
			break;
		}
	}
}

bool
parse_optstring(char *s, void *ctx, parse_optstring_cb cb)
{
	char *osp = s, *k = s, *v = NULL;

	uint32_t i;
	size_t len = strlen(s);
	char os[len + 2];
	memcpy(os, s, len);
	os[len] = os[len + 1] = 0;
	uint8_t set = 1;

	while (*s != 0) {
		switch (*s) {
		case ' ':
			continue;
		case ',':
			if (!v) {
				LOG_W("unexpected ','");
				goto parse_err;
			}

			*s = 0;

			if (!cb(ctx, k, v)) {
				goto parse_err;
			}

			k = v = NULL;
			set = 1;
			break;
		case '=':
			if (v) {
				LOG_W("unexpected '='");
				goto parse_err;
			} else if (set) {
				LOG_W("missing key");
				goto parse_err;
			}

			*s = 0;
			set = 2;
			break;
		default:
			if (set == 1) {
				k = s;
				set = 0;
			} else if (set == 2) {
				v = s;
				set = 0;
			}
		}

		++s;
	}

	if (k && *k) {
		if (v && *v) {
			if (!cb(ctx, k, v)) {
				goto parse_err;
			}
		} else {
			LOG_W("expecting a value");
			goto parse_err;
		}
	}

	return true;

parse_err:
	fprintf(logfile, "%s\n", os);

	for (i = 0; i < len; ++osp, ++i) {
		os[i] = osp == s ? '^' : ' ';
	}

	if (osp == s) {
		os[i] = '^';
	}

	fprintf(logfile, "%s\n", os);

	return false;
}
