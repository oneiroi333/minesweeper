#ifndef GUI_H
#define GUI_H

#include <ncurses.h>
#include <menu.h>
#include "game.h"

/* Menu options */
#define PLAY 0
#define OPTIONS 1
#define QUIT 2

struct gui {
	struct menu {
		WINDOW *menu_win;
		MENU *menu;
	} menu;
	WINDOW *game_win;
	WINDOW *menu_bar_win;
	WINDOW *game_over_win;
	WINDOW *options_win;
};

/*
 * Init ncurses stuff
 */
void gui_init(struct gui *gui);

/*
 * Show main menu screen and handle player input.
 * Return selected menu option.
 */
int gui_menu_show(struct game *game, struct gui *gui);

/*
 * Show the actual game screen and handle player actions.
 */
void gui_game_show(struct game *game, struct gui *gui);

/*
 * Show options screen and handle player selection.
 */
void gui_options_show(struct game *game, struct gui *gui);

/*
 * Destroy all windows and ncurses
 */
void gui_destroy(struct gui *gui);

#endif /* GUI_H */
