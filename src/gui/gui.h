#ifndef GUI_H
#define GUI_H

#include <ncurses.h>
#include <menu.h>
#include <stdint.h>

/* Menu options */
#define OPT_PLAY    0
#define OPT_OPTIONS 1
#define OPT_QUIT    2

struct window {
	WINDOW *win;
	int pos_y;
	int pos_x;
	int width;
	int height;
};

struct gui {
	struct gui_title {
		struct window title_win;
		uint32_t *data;
		int data_len;
	} title;
	struct gui_menu {
		struct window menu_win;
		MENU *menu;
	} menu;
	struct gui_game {
		struct window game_play_win;
		struct window game_over_win;
		struct window game_menu_bar_win;
	} game;
	struct gui_options {
		struct window options_win;
		int grid;
	} options;
};

void gui_init(struct gui *gui);
void gui_run(struct gui *gui);
void gui_destroy(struct gui *gui);

#endif /* GUI_H */
