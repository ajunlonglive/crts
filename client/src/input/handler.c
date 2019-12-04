#include "input.h"

static void stop_threads()
{
	L("stopping threads");
	pthread_cancel(gs.threads.receive);
	pthread_join(gs.threads.receive, NULL);
	L("canceled receive thread");
	pthread_join(gs.threads.respond, NULL);
	L("joined respond thread");
	pthread_cancel(gs.threads.update);
	pthread_join(gs.threads.update, NULL);
	L("canceled update thread");
}

static void handle_input(int key)
{
	switch (gs.mode) {
	case view_mode_normal:
		switch (key) {
		case 'k':
		case KEY_UP:
			gs.view.y -= 4;
			break;
		case 'j':
		case KEY_DOWN:
			gs.view.y += 4;
			break;
		case 'h':
		case KEY_LEFT:
			gs.view.x -= 4;
			break;
		case 'l':
		case KEY_RIGHT:
			gs.view.x += 4;
			break;
		case 's':
			gs.mode = view_mode_select;
			break;
		case 'q':
			gs.run = 0;
			break;
		default:
			break;
		}
		break;
	case view_mode_select:
		switch (key) {
		case 'k':
		case KEY_UP:
			gs.cursor.y -= 4;
			break;
		case 'j':
		case KEY_DOWN:
			gs.cursor.y += 4;
			break;
		case 'h':
		case KEY_LEFT:
			gs.cursor.x -= 4;
			break;
		case 'l':
		case KEY_RIGHT:
			gs.cursor.x += 4;
			break;
		case 'g':
			create_move_to();
			break;
		case 'q':
			gs.mode = view_mode_normal;
			break;
		}

		break;
	}
}

