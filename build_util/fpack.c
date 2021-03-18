#include "posix.h"

/*
 * fpack - read a bitmap font and produce a tga texture atlas
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSIZE 256
#define IMG_MAXH 257
#define BDFCHAR_MAXH 32
#define CHARS_MAX 256

struct opts {
	const char *in_path, *out_path, *out_hdr;
	FILE *in, *out, *outh;
	uint32_t width, height;
};

static struct opts defopts = {
	.out_path = "font-atlas.tga",
	.out_hdr = "font-atlas.h",
	.width = 0, .height = 0,
};

enum bitmap_token_name {
	bt_startchar,
	bt_encoding,
	bt_bitmap,
	bt_endchar,
	bt_dwidth,
};

char *btkn[] = {
	[bt_startchar] = "STARTCHAR",
	[bt_encoding]  = "ENCODING",
	[bt_bitmap]    = "BITMAP",
	[bt_endchar]   = "ENDCHAR",
	[bt_dwidth]    = "DWIDTH",
};

struct bdfchar {
	uint64_t bitmap[BDFCHAR_MAXH];
	size_t encoding;
	uint32_t width;
	int32_t mapi;
};

struct tgawriter {
	FILE *f;
	uint32_t ch, cw;
};

static void
print_usage(void)
{
	fprintf(stderr,
		"usage: fpack [OPTS] my_font.bdf\n"
		"OPTS\n"
		"  -d <height>[x<width>]\n"
		"  -o <outfile>\n"
		"  -r <outhdr>\n"
		"  -h - show help\n"
		);
}

static void
parse_opts(struct opts *opts, const int argc, char * const *argv)
{
	signed char opt;
	char *p;

	*opts = defopts;

	while ((opt = getopt(argc, argv, "d:o:r:h")) != -1) {
		switch (opt) {
		case 'd':
			opts->height = strtol(optarg, NULL, 10);

			if ((p = strchr(optarg, 'x')) && *(++p) != '\0') {
				opts->width = strtol(p, NULL, 10);
			} else {
				opts->width = opts->height;
			}
			break;
		case 'o':
			opts->out_path = optarg;
			break;
		case 'r':
			opts->out_hdr = optarg;
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (optind < argc) {
		opts->in_path = argv[optind];
	} else {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (!(opts->in = fopen(opts->in_path, "r"))) {
		fprintf(stderr, "couldn't open '%s' for reading: %s\n",
			opts->in_path,
			strerror(errno));
		exit(EXIT_FAILURE);
	} else if (!(opts->out = fopen(opts->out_path, "w"))) {
		fprintf(stderr, "couldn't open '%s' for writing: %s\n",
			opts->out_path,
			strerror(errno));
		exit(EXIT_FAILURE);
	} else if (!(opts->outh = fopen(opts->out_hdr, "w"))) {
		fprintf(stderr, "couldn't open '%s' for writing: %s\n",
			opts->out_hdr,
			strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static bool
check_btkn(const char *s, enum bitmap_token_name tkn, const char **tail)
{
	size_t len = strlen(btkn[tkn]);

	if (strncmp(s, btkn[tkn], len) == 0) {
		if (tail) {
			*tail = &s[len + 1];
		}
		return true;
	} else {
		return false;
	}
}

static uint64_t rb_lookup[16] = {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
	0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
};

static uint64_t
reverse_bytes(uint64_t s)
{
	return (rb_lookup[(s >> 0 ) & 0xf] << 60)
	       | (rb_lookup[(s >> 4 ) & 0xf] << 56)
	       | (rb_lookup[(s >> 8 ) & 0xf] << 52)
	       | (rb_lookup[(s >> 12) & 0xf] << 48)
	       | (rb_lookup[(s >> 16) & 0xf] << 44)
	       | (rb_lookup[(s >> 20) & 0xf] << 40)
	       | (rb_lookup[(s >> 24) & 0xf] << 36)
	       | (rb_lookup[(s >> 28) & 0xf] << 32)
	       | (rb_lookup[(s >> 32) & 0xf] << 28)
	       | (rb_lookup[(s >> 36) & 0xf] << 24)
	       | (rb_lookup[(s >> 40) & 0xf] << 20)
	       | (rb_lookup[(s >> 44) & 0xf] << 16)
	       | (rb_lookup[(s >> 48) & 0xf] << 12)
	       | (rb_lookup[(s >> 52) & 0xf] << 8 )
	       | (rb_lookup[(s >> 56) & 0xf] << 4 )
	       | (rb_lookup[(s >> 60) & 0xf] << 0 );
}

static void
parse_bdf_cb(struct bdfchar *chr, size_t *chri, const char *line)
{
	const char *c;
	size_t len;

	if (*chri >= CHARS_MAX) {
		return;
	}

	if (chr[*chri].mapi != 0) {
		if (check_btkn(line, bt_endchar, &c)) {
			//write_bdfchar(bw, chr);
			(*chri)++;
		} else {
			if (chr[*chri].mapi == -1) {
				chr[*chri].mapi = 0;
			} else if (chr[*chri].mapi >= BDFCHAR_MAXH) {
				fprintf(stderr, "warning: trimming tall char\n");
				return;
			}

			len = strlen(line);

			chr[*chri].bitmap[chr[*chri].mapi++] =
				reverse_bytes(strtoul(line, NULL, 16)) >> (64 - (len * 4));

			/*
			   printf("setting bitmap for '%c' | %ld, %d | %02lX\n",
			        (int)chr[*chri].encoding,
			 * chri,
			        chr[*chri].mapi - 1,
			        chr[*chri].bitmap[chr[*chri].mapi - 1]);
			 */
		}
	} else if (check_btkn(line, bt_encoding, &c)) {
		chr[*chri].encoding = strtol(c, NULL, 10);
	} else if (check_btkn(line, bt_dwidth, &c)) {
		chr[*chri].width = strtol(c, NULL, 10);
	} else if (check_btkn(line, bt_bitmap, NULL)) {
		chr[*chri].mapi = -1;
	}
}

static void
write_tga_hdr(struct tgawriter *bw, struct opts *opts)
{
	uint8_t hdr[18] = { 0 };

	hdr[2]  = 2;
	hdr[12] = 255 & opts->width;
	hdr[13] = 255 & (opts->width >> 8);
	hdr[14] = 255 & opts->height;
	hdr[15] = 255 & (opts->height >> 8);
	hdr[16] = 32;
	hdr[17] = 32;

	fwrite(hdr, 1, 18, bw->f);
}

static void
write_bdfchar(const struct tgawriter *bw, const struct opts *opts,
	const struct bdfchar *chr, size_t chars)
{
	uint8_t pixels[IMG_MAXH * IMG_MAXH * 4] = { 0 }, p = 0;
	size_t i, j, ci, cpr = opts->width / bw->cw, packed = 0;
	const struct bdfchar *c;
	uint64_t bm;

	for (i = 0; i < opts->height; ++i) {
		for (j = 0; j < opts->width; ++j) {

			ci = (i / bw->ch * cpr) + j / bw->cw;

			if (ci > chars) {
				continue;
			} else if (j / bw->cw > cpr - 1) {
				continue;
			}

			c = &chr[ci];

			if (c->encoding > 255) {
				continue;
			}

			bm = c->bitmap[i % bw->ch];

			if (i % bw->ch == 0 && j % bw->cw == 0) {
				fprintf(opts->outh, "	[%3ld] = { %ff, %ff },\n",
					c->encoding,
					(double)(j) / opts->width,
					(double)(i) / opts->height
					);
				++packed;
			}

			/*
			   printf("%ld, %ld (%d)\n", ci, i % bw->ch, c->mapi);
			 */

			if (bm & (1 << (j % bw->cw))) {
				p = 255;
			}

			ci = (i * opts->width * 4) + (j * 4);

			pixels[ci + 0] = p;
			pixels[ci + 1] = p;
			pixels[ci + 2] = p;
			pixels[ci + 3] = p;

			p = 0;
		}
	}

	fwrite(pixels, 4, opts->width * opts->height, bw->f);

	fprintf(opts->outh, "};\n#define FONT_ATLAS_LEN %ld\n", packed);
}

static void
parse_bdf(FILE *infile, struct bdfchar *ctx, size_t *chars)
{
	size_t read, i, lo = 0;
	char ibuf[CSIZE] = { 0 }, *a, *b;

	while ((read = fread(&ibuf[lo], sizeof(char), CSIZE - (1 + lo), infile))) {
		read += lo;
		a = ibuf;
		i = 0;

		while ((b = strchr(a, '\n'))) {
			i = b - ibuf;
			*b = '\0';

			parse_bdf_cb(ctx, chars, a);

			a = b + 1;
		}

		if (i < read) {
			lo = read - i - 1;
			memmove(ibuf, a, lo);
		} else {
			lo = 0;
		}
	}
}

static void
write_hdr_hdr(const struct tgawriter *bw, const struct opts *opts)
{
	fprintf(opts->outh,
		"#ifndef GENERATED_FONT_ATLAS_H\n"
		"#define GENERATED_FONT_ATLAS_H\n"
		"#define FONT_ATLAS_WIDTH %d\n"
		"#define FONT_ATLAS_HEIGHT %d\n"
		"#define FONT_ATLAS_CWIDTH %d\n"
		"#define FONT_ATLAS_CHEIGHT %d\n"
		"float font_atlas_cdim[2] = { %ff, %ff };\n"
		"float font_atlas[256][2] = {\n",
		opts->width,
		opts->height,
		bw->cw,
		bw->ch,
		(double)bw->cw / opts->width,
		(double)bw->ch / opts->height
		);
}

int
main(const int argc, char * const*argv)
{
	size_t chars = 0;
	struct bdfchar chr[CHARS_MAX] = { 0 };
	struct opts opts = { 0 };

	parse_opts(&opts, argc, argv);

	parse_bdf(opts.in, chr, &chars);

	printf("processed %ld chars\n", chars);

	struct tgawriter bw = {
		.f = opts.out,
		.cw = chr[0].width,
		.ch = chr[0].mapi,
	};

	if (opts.width == 0 || opts.height == 0) {
		opts.width = 256; //bw.cw * 32;
		opts.height = 256;//bw.ch * (chars / 32) + bw.ch;
	}

	printf("image: %dx%d | char %dx%d\n", opts.height, opts.width, bw.cw, bw.ch);

	write_hdr_hdr(&bw, &opts);

	write_tga_hdr(&bw, &opts);
	write_bdfchar(&bw, &opts, chr, chars);

	fprintf(opts.outh, "#endif\n");

	fclose(opts.in);
	fclose(opts.out);
	fclose(opts.outh);

	return 0;
}
