#ifndef CLIENT_INPUT_CMDLINE_H
#define CLIENT_INPUT_CMDLINE_H

#include <stddef.h>
#include <stdint.h>

#define MAX_ARGC 32

#define CMDLINE_BUF_LEN 256
#define CMDLINE_HIST_LEN 32

enum cmd_result {
	cmdres_ok,
	cmdres_not_found,
	cmdres_arg_error,
	cmdres_cmd_error,
	cmdres_dont_keep_hist,
};

struct cmd_ctx {
	char cmdline[CMDLINE_BUF_LEN];
	char *argv[MAX_ARGC];
	uint32_t argc;
	char out[CMDLINE_BUF_LEN];
	enum cmd_result res;
};

typedef enum cmd_result ((*cmdfunc)(struct cmd_ctx *cmd, void *ctx));

struct cmd_table {
	char *cmd;
	cmdfunc action;
};

struct cmdline_buf {
	char buf[CMDLINE_BUF_LEN];
	size_t len, cursor;
};


struct cmdline {
	struct cmdline_buf cur, tmp;
	struct {
		char in[CMDLINE_HIST_LEN][CMDLINE_BUF_LEN];
		char out[CMDLINE_HIST_LEN][CMDLINE_BUF_LEN];
		size_t len, cursor;
	} history;
};

struct client;

void parse_cmd_input(struct client *cli, unsigned k);
cmdfunc cmd_lookup(const struct cmd_ctx *cmd, const struct cmd_table *tbl, size_t tbl_len);
void run_cmd_string(struct client *cli, const char *cmds);
#endif
