#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct {
	size_t row_len;
	char *base;
	bool header;
} opts = {
	.row_len = 12,
	.base = ""
};

static void
print_usage(void)
{
	printf("usage: embed_binary [OPTS] [infile [infile [...]] > out.c\n\n"
		"OPTS:\n"
		"-l <integer> - set number items per row\n"
		"-h           - show help\n"
		);
}

static void
print_header(void)
{
	printf(
		"#ifndef EMBEDDED_DATA_H\n"
		"#define EMBEDDED_DATA_H\n"
		"\n"
		"#include <stddef.h>\n"
		"#include <stdint.h>\n"
		"\n"
		"#include \"shared/types/hash.h\"\n"
		"\n"
		);
}

static void
print_footer(void)
{
	printf(
		"#endif\n"
		);
}

int
main(int argc, char *const * argv)
{
	signed char opt;
	size_t num = 0, i, totals[256];
	int argon;
	FILE *f;


	while ((opt = getopt(argc, argv, "l:h")) != -1) {
		switch (opt) {
		case 'l':
			opts.row_len = strtoul(optarg, NULL, 10);
			break;
		case 'h':
			print_usage();
			return 0;
		default:
			print_usage();
			return 1;

		}
	}

	print_header();

	for (argon = optind; argon < argc; ++argon) {
		if (!(f = fopen(argv[argon], "r"))) {
			fprintf(stderr, "failed to read file '%s': %s\n",
				argv[argon], strerror(errno));
			return 1;
		}

		printf("uint8_t data_%ld[] = { /* %s */\n\t", num, argv[argon]);

		unsigned char buf[BUFSIZ] = { 0 };
		size_t b, i, j = 0, total = 0;
		while ((b = fread(buf, 1, BUFSIZ, f)) > 0) {
			for (i = 0; i < b; ++i) {
				assert(buf[i] <= 0xff);

				printf("0x%02x, ", buf[i]);
				if (opts.row_len > 0 && !(++j % opts.row_len)) {
					printf("\n\t");
				}
			}
			total += b;
		}

		totals[num] = total;

		printf("\n};\n");
		++num;
		fclose(f);
	}

	printf("static struct file_data embedded_files[] = {\n");
	for (i = 0; i < num; ++i) {
		printf("\t{ \"%s\", data_%ld, %ld },\n",
			basename(argv[optind + i]), i, totals[i]);

	}
	printf("};\nstatic size_t embedded_files_len = %ld;\n", num);
	print_footer();
}
