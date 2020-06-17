#include <assert.h>
#include <string.h>

#include "shared/util/log.h"
#include "shared/util/assets.h"
#include "shared/util/text.h"

static void
line_cb(void *ctx, char *line, size_t len)
{
	size_t *i = ctx;

	assert(len == strlen(line));
	printf("%3ld %s\n",  ++(*i), line);
}

int
main(int argc, char *const *argv)
{
	size_t i = 0;
	struct file_data *fd;

	log_level = ll_quiet;
	asset_path_init("");

	if (argc > 1 && (fd = asset(argv[1]))) {
		each_line(fd, &i, line_cb);
	} else {
		return 1;
	}
}
