#ifndef GUI_H
#define GUI_H

#include <ncurses.h>
#include <menu.h>
#include "game.h"

/* Menu options */
#define PLAY 0
#define OPTIONS 1
#define QUIT 2

/*
 * Init ncurses stuff
 */
void gui_init(void);

/*
 * Show main menu screen and handle player input.
 * Return selected menu option.
 */
int gui_main_scr(struct game *game);

/*
 * Show the actual game screen and handle player actions.
 */
void gui_game_scr(struct game *game);

/*
 * Show options screen and handle player selection.
 */
void gui_options_scr(struct game *game);

/*
 * Destroy all ncurses stuff
 */
void gui_destroy(void);

#endif /* GUI_H */
