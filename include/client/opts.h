#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#include <stdbool.h>
#include <stdint.h>

struct client_opts {
	struct {
		uint32_t device;
		float master;
		float music;
		float sfx;
		bool disable;
	} sound;

	struct {
		float ui_scale;
		bool shadows;
		bool water;
		float mouse_sensitivity;
	} ui_cfg;

	const char *cmds;
	uint16_t id;
	uint8_t ui;

	const char *keymap;
};

void parse_client_opts(int argc, char * const *argv, struct client_opts *opts);
#endif
