#ifndef CLIENT_UI_NCURSES_CONTAINER_H
#define CLIENT_UI_NCURSES_CONTAINER_H
struct display_container {
	struct win *_root;
	struct {
		struct win *_info;
		struct {
			struct win *l;
			struct win *r;
		} info;
		struct win *world;
	} root;
};

void dc_init(struct display_container *dc);
#endif
