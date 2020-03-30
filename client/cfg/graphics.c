#include <stdlib.h>
#include <string.h>

#include "client/cfg/cfg.h"
#include "client/cfg/graphics.h"
#include "client/cfg/ini.h"
#include "client/display/window.h"
#include "client/graphics.h"
#include "shared/util/log.h"

#define DELIM ", "
#define BUFLEN 128
#define VAL_FIELDS 5
#define VAL_INT_BASE 16

enum section {
	section_global,
	section_tiles,
	section_entities,
	section_cursor,
};

static struct lookup_table ltbl[] =  {
	[section_global] = {
		"tiles", section_tiles,
		"entities", section_entities,
		"cursor", section_cursor,
	},
	[section_tiles] = {
		"deep_water", tile_deep_water,
		"water", tile_water,
		"coral", tile_coral,
		"wetland", tile_wetland,
		"wetland_forest_young", tile_wetland_forest_young,
		"wetland_forest", tile_wetland_forest,
		"wetland_forest_old", tile_wetland_forest_old,
		"plain", tile_plain,
		"dirt", tile_dirt,
		"forest_young", tile_forest_young,
		"forest", tile_forest,
		"forest_old", tile_forest_old,
		"mountain", tile_mountain,
		"peak", tile_peak,
		"wood", tile_wood,
		"wood_floor", tile_wood_floor,
		"rock_floor", tile_rock_floor,
		"stone", tile_stone,
		"shrine", tile_shrine,
		"farmland_empty", tile_farmland_empty,
		"farmland_done", tile_farmland_done,
		"fire", tile_burning,
		"ashes", tile_burnt,
	},
	[section_entities] = {
		"elf_friend", et_elf_friend,
		"elf_foe", et_elf_foe,
		"wood", et_resource_wood,
		"rock", et_resource_rock,
		"deer", et_deer,
		"fish", et_fish,
		"meat", et_resource_meat,
		"crop", et_resource_crop,
		"elf_corpse", et_elf_corpse,
	},
	[section_cursor] = {
		"default", ct_default,
		"blueprint_valid", ct_blueprint_valid,
		"blueprint_invalid", ct_blueprint_invalid,
		"crosshair", ct_crosshair,
		"crosshair_dim", ct_crosshair_dim,
		"harvest", ct_harvest,
	}
};

static struct graphics_info_t *
parse_section_key(struct graphics_t *g, const char *sec, const char *key, int32_t *item)
{
	enum section s;
	struct graphics_info_t *ret;

	switch (s = cfg_string_lookup(sec, &ltbl[section_global])) {
	case section_tiles:
		ret = g->tiles;
		break;
	case section_entities:
		ret = g->entities;
		break;
	case section_cursor:
		ret = g->cursor;
		break;
	default:
		return NULL;
	}

	*item = cfg_string_lookup(key, &ltbl[s]);
	return ret;
}

static bool
parse_graphics_char(const char *v, char *c)
{
	size_t len = strlen(v);

	if (v == NULL) {
		L("out of tokens");
		return false;
	}

	if (v[0] == '\\') {
		if (len < 2) {
			L("unterminated escape sequence");
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
			L("invalid escape sequence");
			return false;
		}
	} else {
		*c = v[0];
	}

	return true;
}

static bool
parse_graphics_val(const char *v, char *c, short *fg, short *bg,
	short *attr, short *zi)
{
	char buf[BUFLEN] = { 0 };
	char *bufp = buf, *tok[VAL_FIELDS];
	size_t i = strlen(v);

	if (i >= BUFLEN) {
		L("string too long");
		return false;
	}

	strncpy(buf, v, i);

	for (i = 0; i < VAL_FIELDS; ++i) {
		if ((tok[i] = strtok(bufp, DELIM)) == NULL) {
			L("out of tokens, got %ld, expected %d", i, VAL_FIELDS);
			return false;
		}

		bufp = NULL;
	}

	if (!parse_graphics_char(tok[0], c)) {
		return false;
	}

	*fg = strtol(tok[1], NULL, VAL_INT_BASE);
	if (*fg < -1 || *fg > 0xff) {
		L("invalid fg color");
		return false;
	}

	*bg = strtol(tok[2], NULL, VAL_INT_BASE);
	if (*bg < -2 || *bg > 0xff) {
		L("invalid bg color");
		return false;
	}

	*attr = strtol(tok[3], NULL, VAL_INT_BASE);
	if (*attr < 0 || *attr > 0xff) {
		L("invalid attrs");
		return false;
	}

	*zi = strtol(tok[4], NULL, VAL_INT_BASE);
	if (*zi < 0 || *zi > z_index_count) {
		L("invalid z-index");
		return false;
	}

	return true;
}

int
parse_graphics_handler(void *_g, const char *sect, const char *k, const char *v, int line)
{
	struct graphics_t *g = _g;
	struct graphics_info_t *cat;
	int32_t type = -1;
	short fg = 0, bg = 0, attr = 0, zi = 0;
	uint32_t clr;
	char c;

	assert(v != NULL);
	assert(k != NULL);

	if (sect == NULL || !(cat = parse_section_key(g, sect, k, &type))) {
		L("invalid section '%s' while parsing graphics at line %d", sect, line);
		return false;
	} else if (type < 0) {
		L("invalid item '%s' while parsing graphics at line %d", k, line);
		return false;
	} else if (!parse_graphics_val(v, &c, &fg, &bg, &attr, &zi)) {
		L("invalid value for '%s' while parsing graphics at line %d", k, line);
		return false;
	}

	clr = setup_color_pair(g, fg, bg);

	if (bg == TRANS_COLOR) {
		if (zi == 0) {
			L("zi must be > 0 for transparent bg");
			return false;
		}

		g->trans_bg.fg_map[fg + TRANS_COLOR_BUF] = g->trans_bg.fgi++;

		if (g->trans_bg.fgi >= TRANS_COLORS) {
			L("too many transparent backgrounds");
			return false;
		}
	}

	cat[type].pix.c = c;
	cat[type].pix.clr = clr;
	cat[type].pix.attr = attr_transform(attr);
	cat[type].pix.fg = fg;
	cat[type].pix.bg = bg;
	cat[type].zi = zi;

	return true;
}
