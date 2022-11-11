#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/cmdline.h"
#include "client/input_handler.h"
#include "shared/sim/action.h"
#include "shared/util/log.h"

#ifndef NDEBUG

#include "shared/pathfind/api.h"
#include "shared/pathfind/preprocess.h"

void
debug_pathfind_toggle(struct client *cli, uint32_t _)
{
	if ((cli->debug_path.on = !cli->debug_path.on)) {
		ag_init_components(&cli->world->chunks);

		struct point c = cli->cursor;
		cli->debug_path.goal = c;
		L(log_misc, "adding goal @ %d, %d", c.x, c.y);
	}

	cli->changed.chunks = true;
}

void
debug_pathfind_place_point(struct client *cli, uint32_t _)
{
	if (!cli->debug_path.on) {
		return;
	}

	struct point c = cli->cursor;

	if (!hpa_start(&cli->world->chunks, &c, &cli->debug_path.goal, &cli->debug_path.path)) {
		return;
	}

	darr_clear(&cli->debug_path.path_points);

	darr_push(&cli->debug_path.path_points, &c);

	uint32_t i, duplicates = 0;

	while ((hpa_continue(&cli->world->chunks, cli->debug_path.path, &c)) == rs_cont) {
		for (i = 0; i < darr_len(&cli->debug_path.path_points); ++i) {
			struct point *d = darr_get(&cli->debug_path.path_points, i);
			if (points_equal(&c, d)) {
				++duplicates;
			}
		}

		darr_push(&cli->debug_path.path_points, &c);
	}

	L(log_misc, "duplicates in path: %d", duplicates);

	cli->changed.chunks = true;
}
#endif

void
set_action_type(struct client *cli, uint32_t id)
{
	if (id >= action_count) {
		return;
	}

	cli->action = id;
}

void
set_ent_type(struct client *cli, uint32_t id)
{
	if (id >= ent_type_count) {
		return;
	}

	cli->ent_type = id;
}

#define DEF_MOVE_AMNT 1

void
center(struct client *d, uint32_t _)
{
	/* d->view.x = 0; */
	/* d->view.y = 0; */
}

void
center_cursor(struct client *cli, uint32_t _)
{
	/* cli->view.x += cli->cursor.x - cli->viewport.width / 2; */
	/* cli->view.y += cli->cursor.y - cli->viewport.height / 2; */
	/* cli->cursor.x = cli->viewport.width / 2; */
	/* cli->cursor.y = cli->viewport.height / 2; */
}

static void
move_base(struct client *cli, float dx, float dy, struct pointf *p)
{
	dx /= cli->ref.w;
	dy /= cli->ref.h;

	struct pointf v1 = {
		(cli->ref.rect.p[3].x - cli->ref.rect.p[0].x) * dy,
		(cli->ref.rect.p[3].y - cli->ref.rect.p[0].y) * dy,
	}, v2 = {
		(cli->ref.rect.p[1].x - cli->ref.rect.p[0].x) * dx,
		(cli->ref.rect.p[1].y - cli->ref.rect.p[0].y) * dx,
	};

	p->x -= v1.x + v2.x;
	p->y -= v1.y + v2.y;

}

static void
copy_cursor(struct client *cli)
{
	cli->cursor.x = cli->cursorf.x + 0.5f;
	cli->cursor.y = cli->cursorf.y + 0.5f;
}

static void
cursor_up(struct client *cli, uint32_t num)
{
	move_base(cli, 0, 0.0f - (float)num, &cli->cursorf);
	copy_cursor(cli);
}

static void
cursor_down(struct client *cli, uint32_t num)
{
	move_base(cli, 0, num, &cli->cursorf);
	copy_cursor(cli);
}

static void
cursor_left(struct client *cli, uint32_t num)
{
	move_base(cli, 0.0f - (float)num, 0, &cli->cursorf);
	copy_cursor(cli);
}

static void
cursor_right(struct client *cli, uint32_t num)
{
	move_base(cli, num, 0, &cli->cursorf);
	copy_cursor(cli);
}

static void
view_up(struct client *cli, uint32_t num)
{
	move_base(cli, 0, 0.0f - (float)num, &cli->ref.center);
}

static void
view_down(struct client *cli, uint32_t num)
{
	move_base(cli, 0, num, &cli->ref.center);
}

static void
view_left(struct client *cli, uint32_t num)
{
	move_base(cli, 0.0f - (float)num, 0, &cli->ref.center);
}

static void
view_right(struct client *cli, uint32_t num)
{
	move_base(cli, num, 0, &cli->ref.center);
}

void
constrain_cursor(struct client *cli)
{
	return; // XX

	const float scale = 0.5f;
	struct rect r;
	make_rotated_rect(&cli->ref.center, cli->ref.h * scale, cli->ref.w * scale, cli->ref.angle, &r);

	if (point_in_rect(&cli->cursor, &r)) {
		return;
	}

	struct pointf *a, *b, c;
	line cline, rline;
	make_line(&cli->ref.center, &cli->cursorf, cline);

	L(log_cli, "---");
	uint32_t i;
	for (i = 0; i < 4; ++i) {
		a = &r.p[i];
		b = &r.p[(i + 1) & 3];
		L(log_cli, "> line (%f, %f, | %f, %f)", a->x, a->y, b->x, b->y);
		make_line(a, b, rline);

		float det = cline[0] * rline[1] - cline[1] * rline[0];

		if (det == 0) {
			L(log_cli, ">> parallel");
			// parallel
			continue;
		}
		L(log_cli, ">> intersecting!");

		c.x = (rline[1] * cline[2] - cline[1] * rline[2]) / det;
		c.y = (cline[0] * rline[2] - rline[0] * cline[2]) / det;

		float dac = a->x * c.x + a->y * c.y;

		if (dac < 0) {
			L(log_cli, ">> off segment 1");
			continue;
		}

		float dab = a->x * b->x + a->y * b->y;

		if (dac > dab) {
			L(log_cli, ">> off segment 2");
			continue;
		}

		cli->cursorf = c;
		copy_cursor(cli);
		break;
	}
}

static void
do_nothing(struct client *_, uint32_t __)
{
}

static void
end_simulation(struct client *cli, uint32_t _)
{
	cli->run = 0;
}

static void
pause_simulation(struct client *cli, uint32_t _)
{
	cli->state ^= csf_paused;

	enum server_cmd cmd;
	if (cli->state & csf_paused) {
		cmd = server_cmd_pause;
	} else {
		cmd = server_cmd_unpause;
	}

	msgr_queue(cli->msgr, mt_server_cmd, &(struct msg_server_cmd) {
		.cmd = cmd,
	}, 0, priority_normal);
}

static void
set_input_mode(struct client *cli, uint32_t im)
{
	if (im < input_mode_count) {
		cli->im = im;
	}
}

#define MAX_REGISTERED_COMMAND_NAMES 4
#define MAX_REGISTERED_CONSTANTS 4
static struct {
	const struct input_command_name *command_names[MAX_REGISTERED_COMMAND_NAMES];
	uint32_t command_names_len;

	const struct cfg_lookup_table *constants[MAX_REGISTERED_CONSTANTS];
	uint32_t constants_len;
} registry;

void
register_input_commands(const struct input_command_name *alt)
{
	if (registry.command_names_len >= MAX_REGISTERED_COMMAND_NAMES) {
		assert(false && "too many input commands registered");
	}

	registry.command_names[registry.command_names_len] = alt;
	++registry.command_names_len;
}

void
register_input_constants(const struct cfg_lookup_table *ltbl)
{
	if (registry.constants_len >= MAX_REGISTERED_CONSTANTS) {
		assert(false && "too many constants registered");
	}

	registry.constants[registry.constants_len] = ltbl;
	++registry.constants_len;
}

bool
input_command_name_lookup(const char *name, kc_func *res)
{
	uint32_t i, j;
	for (i = 0; i < registry.command_names_len; ++i) {
		for (j = 0; registry.command_names[i][j].name; ++j) {
			if (strcmp(name, registry.command_names[i][j].name) == 0) {
				*res = registry.command_names[i][j].func;
				return true;
			}
		}
	}

	return false;
}

bool
input_constant_lookup(const char *name, uint32_t *res)
{
	uint32_t i;
	int32_t r;
	for (i = 0; i < registry.constants_len; ++i) {
		if ((r = cfg_string_lookup(name, registry.constants[i])) != -1) {
			*res = r;
			return true;
		}
	}

	return false;
}

struct keymaps {
	struct darr layers;
	uint32_t cur_layer;
};

static struct keymaps kms = { 0 };

void
km_set_layer(uint8_t l)
{
	kms.cur_layer = l;
}

void
km_add_layer(uint8_t *res)
{
	assert(kms.layers.len <= UINT8_MAX);
	*res = kms.layers.len;
	kms.cur_layer = darr_push(&kms.layers, &(struct keymap_layer) { 0 });
}

struct keymap_layer *
km_current_layer(void)
{
	return darr_get(&kms.layers, kms.cur_layer);
}

struct keymap *
km_find_map(uint8_t key, uint8_t mod, uint8_t action)
{
	uint8_t i;
	struct keymap_layer *layer = km_current_layer();

	for (i = 0; i < layer->len; ++i) {
		if (layer->maps[i].key == key
		    && layer->maps[i].mod == mod) {
			if (layer->maps[i].action == action) {
				return &layer->maps[i];
			}
		}
	}

	return NULL;
}

struct keymap *
km_find_or_create_map(struct keymap_layer *layer, uint8_t key, uint8_t mod, uint8_t action)
{
	uint8_t i;

	for (i = 0; i < layer->len; ++i) {
		if (layer->maps[i].key == key
		    && layer->maps[i].mod == mod
		    && layer->maps[i].action == action) {
			return &layer->maps[i];
		}
	}

	if (layer->len >= LAYER_MAX) {
		return NULL;
	}

	i = layer->len;
	++layer->len;

	layer->maps[i].key = key;
	layer->maps[i].mod = mod;
	layer->maps[i].action = action;
	return &layer->maps[i];
}

void
input_init(void)
{
#if defined(NDEBUG)
#define DEBUG_CMD(_) do_nothing
#else
#define DEBUG_CMD(cmd) cmd
#endif
	static const struct input_command_name core_input_command_names[] = {
		{ "none", do_nothing },
		{ "invalid", do_nothing },
		{ "macro", do_nothing },
		{ "center_cursor", center_cursor },
		{ "set_input_mode", set_input_mode },
		{ "quit", end_simulation },
		{ "cursor_up", cursor_up },
		{ "cursor_down", cursor_down },
		{ "cursor_left", cursor_left },
		{ "cursor_right", cursor_right },
		{ "view_up", view_up },
		{ "view_down", view_down },
		{ "view_left", view_left },
		{ "view_right", view_right },
		{ "set_action_type", set_action_type },
		{ "set_ent_type", set_ent_type },
		{ "pause", pause_simulation },
		{ "debug_pathfind_toggle", DEBUG_CMD(debug_pathfind_toggle) },
		{ "debug_pathfind_place_point", DEBUG_CMD(debug_pathfind_place_point) },
		{ 0 },
	};

	static const struct cfg_lookup_table core_input_constants = {
		"im_normal", im_normal,
		"im_cmd", im_cmd,
		"neutral", act_neutral,
		"create", act_create,
		"destroy", act_destroy,
		"raise", act_raise,
		"lower", act_lower,
		"fire", et_fire,
		"sand", et_sand,
		"wood", et_wood,
		"acid", et_acid,
		"water", et_water,
		"spring", et_spring,
		"explosion", et_explosion,
	};

	register_input_commands(core_input_command_names);
	register_input_constants(&core_input_constants);

	darr_init(&kms.layers, sizeof(struct keymap_layer));
	uint8_t l;
	km_add_layer(&l);
}

void
input_handle_key(struct client *cli, uint8_t key, uint8_t mod, enum key_action action)
{
	switch (cli->im) {
	case im_normal: {
		struct keymap *m;

		if ((m = km_find_map(key, mod, action))) {
			cli->changed.input = true;
			if (m->func) {
				m->func(cli, m->arg);
				kms.cur_layer = 0;
			} else {
				kms.cur_layer = m->next_layer;
			}
		} else {
			kms.cur_layer = 0;
		}
		break;
	}
	case im_cmd: {
		cmdline_input_handle(cli, key, mod);
		break;
	}
	default:
		break;
	}
}

void
input_handle_mouse(struct client *cli, float dx, float dy)
{
	move_base(cli, dx, dy, &cli->cursorf);
	copy_cursor(cli);
}
