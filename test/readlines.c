#include <assert.h>
#include <string.h>

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
	FILE *f;
	size_t i = 0;

	if (argc > 1 && (f = fopen(argv[1], "r"))) {
		each_line(f, &i, line_cb);
	} else {
		return 1;
	}
}
