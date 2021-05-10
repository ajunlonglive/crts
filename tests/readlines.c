#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/util/log.h"
#include "shared/util/assets.h"
#include "shared/util/text.h"

static enum iteration_result
line_cb(void *ctx, char *line, size_t len)
{
	size_t *i = ctx;

	assert(len == strlen(line));
	printf("%3ld %s\n",  ++(*i), line);

	return ir_cont;
}

int
main(int argc, char *const *argv)
{
	size_t i = 0;
	struct file_data *fd;

	log_init();
	log_level = ll_debug;
	asset_path_init("");

	if (argc > 1 && (fd = asset(argv[1]))) {
		each_line(fd, &i, line_cb);
	} else {
		return 1;
	}
}
