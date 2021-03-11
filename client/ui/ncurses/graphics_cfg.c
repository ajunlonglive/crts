#include "posix.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/ncurses/graphics.h"
#include "client/ui/ncurses/graphics_cfg.h"
#include "shared/sim/tiles.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

#define DELIM ", "
#define BUFLEN 128
#define VAL_FIELDS 5
#define VAL_INT_BASE 16

static const struct cfg_lookup_table ltbl[] =  {
	[gfx_cfg_section_global] = {
		"tiles", gfx_cfg_section_tiles,
		"entities", gfx_cfg_section_entities,
		"cursor", gfx_cfg_section_cursor,
	},
	[gfx_cfg_section_entities] = {
		"elf_friend", et_elf_friend,
		"elf_foe", et_elf_foe,
		"wood", et_resource_wood,
		"rock", et_resource_rock,
		"deer", et_deer,
		"fish", et_fish,
		"meat", et_resource_meat,
		"crop", et_resource_crop,
		"elf_corpse", et_elf_corpse,
		"boat", et_vehicle_boat,
	},
	[gfx_cfg_section_cursor] = {
		"default", ct_default,
		"blueprint_valid", ct_blueprint_valid,
		"blueprint_invalid", ct_blueprint_invalid,
		"crosshair", ct_crosshair,
		"crosshair_dim", ct_crosshair_dim,
		"harvest", ct_harvest,
	}
};

static bool
parse_section_key(char *err, const char *sec, const char *key, int32_t *sect, int32_t *item)
{
	if ((*sect = cfg_string_lookup(sec, &ltbl[gfx_cfg_section_global])) < 0) {
		INIH_ERR("invalid section '%s'", sec);
		return false;
	}

	const struct cfg_lookup_table *tbl;

	if (*sect == gfx_cfg_section_tiles) {
		tbl = &ltbl_tiles;
	} else {
		tbl = &ltbl[*sect];
	}

	if ((*item = cfg_string_lookup(key, tbl)) < 0) {
		INIH_ERR("invalid item '%s'", key);
		return false;
	} else {
		return true;
	}
}

static bool
parse_graphics_char(char *err, const char *v, char *c)
{
	size_t len = strlen(v);

	if (v == NULL) {
		INIH_ERR("parsing char: out of tokens");
		return false;
	}

	if (v[0] == '\\') {
		if (len < 2) {
			INIH_ERR("unterminated escape sequence");
			return false;
		}

		switch (v[1]) {
		case 't':
			*c = CHAR_TRANS;
			break;
		case 's':
			*c = ' ';
			break;
		case 'c':
			*c = ',';
			break;
		case '\\':
			*c = '\\';
			break;
		default:
			INIH_ERR("invalid escape sequence");
			return false;
		}
	} else {
		*c = v[0];
	}

	return true;
}

static bool
parse_graphics_val(char *err, const char *v, char *c, short *fg, short *bg,
	short *attr, short *zi)
{
	char buf[BUFLEN] = { 0 };
	char *bufp = buf, *tok[VAL_FIELDS];
	size_t i = strlen(v);

	if (i >= BUFLEN) {
		INIH_ERR("string too long");
		return false;
	}

	strncpy(buf, v, BUFLEN);

	for (i = 0; i < VAL_FIELDS; ++i) {
		if ((tok[i] = strtok(bufp, DELIM)) == NULL) {
			INIH_ERR("out of tokens, got %ld, expected %d", i, VAL_FIELDS);
			return false;
		}

		bufp = NULL;
	}

	if (!parse_graphics_char(err, tok[0], c)) {
		return false;
	}

	*fg = strtol(tok[1], NULL, VAL_INT_BASE);
	if (*fg < -1 || *fg > 0xff) {
		INIH_ERR("invalid fg color");
		return false;
	}

	*bg = strtol(tok[2], NULL, VAL_INT_BASE);
	if (*bg < -2 || *bg > 0xff) {
		INIH_ERR("invalid bg color");
		return false;
	}

	*attr = strtol(tok[3], NULL, VAL_INT_BASE);
	if (*attr < 0 || *attr > 0xff) {
		INIH_ERR("invalid attrs");
		return false;
	}

	*zi = strtol(tok[4], NULL, VAL_INT_BASE);
	if (*zi < 0 || *zi > z_index_count) {
		INIH_ERR("invalid z-index");
		return false;
	}

	return true;
}

bool
parse_graphics_handler(void *_ctx, char *err, const char *sect, const char *k,
	const char *v, uint32_t line)
{
	struct parse_graphics_ctx *ctx = _ctx;
	int32_t type_e, sect_e;
	short fg = 0, bg = 0, attr = 0, zi = 0;
	char c;

	assert(v != NULL);
	assert(k != NULL);

	if (sect == NULL || !parse_section_key(err, sect, k, &type_e, &sect_e)) {
		return false;
	} else if (!parse_graphics_val(err, v, &c, &fg, &bg, &attr, &zi)) {
		return false;
	} else if (!ctx->setup_color(ctx->ctx, type_e, sect_e, c, fg, bg, attr, zi)) {
		INIH_ERR("failed setting up color");
		return false;
	}

	return true;
}
