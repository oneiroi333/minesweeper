#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gui.h"

/* Enter key code */
#ifdef KEY_ENTER
#undef KEY_ENTER
#endif
#define KEY_ENTER 0xA

/* options: exit with or w/o save */
#define OPT_EXIT_SAVE 0
#define OPT_EXIT_NOSAVE 1

/* Surface field symbols */
#define FIELD_DEFAULT L'\u2591'
#define FIELD_FLAG L'\u2690'

/* Grid symbols */
#define GRID_T L'\u252c'	/* top */
#define GRID_TL L'\u250c'	/* top-left */
#define GRID_TR L'\u2510'	/* top-right */
#define GRID_B L'\u2534'	/* bottom */
#define GRID_BL L'\u2514'	/* bottom-left */
#define GRID_BR L'\u2518'	/* bottom-right */
#define GRID_L L'\u251c'	/* left */
#define GRID_R L'\u2524'	/* right */
#define GRID_X L'\u253c'	/* intersection */
#define GRID_H L'\u2500'	/* vertical */
#define GRID_V L'\u2502'	/* horizontal */

/* Color pairs */
#define COLOR_MAIN_TITLE 1
#define COLOR_GAME_CURSOR 2

#define X_CENTER(container, obj) \
	((container / 2) - (obj / 2))


// TODO
// this values should be set in game_init() in relation to the 'struct title'!!!
/* title */
int title_height = 10;
int title_width = 126;
int title_margin_top = 3;
int title_margin_bottom = 3;
//int offset_top = title_height + title_margin_top + title_margin_bottom;
int offset_top = 16;


static void menu_size(MENU *menu, int *height, int *width);
static void print_title(struct game *game, WINDOW *win, int row, int col);
static void gui_game_print_field(WINDOW *win, int row, int col, int field_val);

void
gui_init(void)
{
	initscr();					/* Init curses mode */
	noecho();					/* Dont show input data */
	curs_set(0);					/* Make cursor invisible */
	cbreak();					/* Line buffering disabled, pass every char on */
	start_color();					/* Enable colors */
	keypad(stdscr, TRUE);				/* Enable the keypad */
	/* Init color pairs */
	init_pair(COLOR_MAIN_TITLE, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_GAME_CURSOR, COLOR_RED, COLOR_BLACK);
}

int
gui_menu_show(struct game *game, struct gui *gui)
{
	WINDOW *menu_win, *menu_sub_win;
	MENU *menu;
	ITEM **items;
	int menu_height, menu_width, menu_y_start, menu_x_start;
	int input, choice, i;
	char *mark = "";

	menu_win = gui->menu.menu_win;
	menu = gui->menu.menu;
	if (!menu_win) {
		/* Print title */
		wattron(stdscr, COLOR_PAIR(COLOR_MAIN_TITLE));
		print_title(game, stdscr, title_margin_top, X_CENTER(COLS, title_width));
		wattroff(stdscr, COLOR_PAIR(COLOR_MAIN_TITLE));
		mvwprintw(stdscr, 1, X_CENTER(COLS, title_width), "%s", "F1: QUIT TO MENU");

		wrefresh(stdscr);

		/* Create the menu */
		items = (ITEM **) malloc(4 * sizeof(ITEM *));
		items[0] = new_item("PLAY", "");
		items[1] = new_item("OPTIONS", "");
		items[2] = new_item("QUIT", "");
		items[3] = (ITEM *) NULL;
		menu = new_menu(items);
		set_menu_spacing(menu, 0, 2, 0);		/* Line spacing */
		menu_size(menu, &menu_height, &menu_width);
		menu_y_start = offset_top;
		menu_x_start = X_CENTER(COLS, menu_width);
		menu_win = newwin(menu_height, menu_width, menu_y_start, menu_x_start);
		menu_sub_win = derwin(menu_win, menu_height, menu_width, 0, 0);
		set_menu_win(menu, menu_win);
		set_menu_sub(menu, menu_sub_win);
		set_menu_mark(menu, mark);
		post_menu(menu);
	}
	wrefresh(menu_win);

	/* Menu selection */
	for(;;) {
		input = getch();
		switch(input) {
		case KEY_DOWN:
			menu_driver(menu, REQ_DOWN_ITEM);
			wrefresh(menu_win);
			break;
		case KEY_UP:
			menu_driver(menu, REQ_UP_ITEM);
			wrefresh(menu_win);
			break;
		case KEY_ENTER:
			choice = item_index(current_item(menu));
			werase(menu_win);
			wrefresh(menu_win);
			return choice;
		}
	}
}

void
gui_game_show(struct game *game, struct gui *gui)
{
	WINDOW *game_win;
	int game_win_height, game_win_width;
	int row, col, row_m, col_m;
	int input;
	uint32_t field_val;

	game_win = gui->game_win;
	/* account for border */
	game_win_height = (game->cfg.rows * 2) + 1;
	game_win_width = (game->cfg.columns * 2) + 1;
	if (!game_win) {
		game_win = newwin(game_win_height, game_win_width, offset_top, X_CENTER(COLS, game_win_width));
	}

	row = col = 0;
	row_m = col_m = 0;
#ifdef GRID
	/* Print top border */
	mvwprintw(game_win, row, col, "%lc", GRID_TL);
	for (col = 1; col < game_win_width - 1; ++col) {
		if (col % 2) {
			mvwprintw(game_win, row, col, "%lc", GRID_H);
		} else {
			mvwprintw(game_win, row, col, "%lc", GRID_T);
		}
	}
	mvwprintw(game_win, row++, col, "%lc", GRID_TR);
#endif
	/* Print the middle part */
	for (; row < game_win_height - 1; ++row) {
		if (row % 2) {
			for (col = 0; col < game_win_width; ++col) {
				if (col % 2) {
					field_val = game_get_value(game, row_m, col_m);
					gui_game_print_field(game_win, row, col, field_val);
					++col_m;
				} else {
#ifdef GRID
					mvwprintw(game_win, row, col, "%lc", GRID_V);
#endif
				}
			}
			col_m = 0;
			++row_m;
		} else {
#ifdef GRID
			for (col = 0; col < game_win_width; ++col) {
				if (col == 0) {
					mvwprintw(game_win, row, col, "%lc", GRID_L);
				} else if (col == game_win_width - 1) {
					mvwprintw(game_win, row, col, "%lc", GRID_R);
				} else if (col % 2) {
					mvwprintw(game_win, row, col, "%lc", GRID_H);
				} else {
					mvwprintw(game_win, row, col, "%lc", GRID_X);
				}
			}
#endif
		}
	}
#ifdef GRID
	/* Print bottom border */
	col = 0;
	mvwprintw(game_win, row, col, "%lc", GRID_BL);
	for (col = 1; col < game_win_width - 1; ++col) {
		if (col % 2) {
			mvwprintw(game_win, row, col, "%lc", GRID_H);
		} else {
			mvwprintw(game_win, row, col, "%lc", GRID_B);
		}
	}
	mvwprintw(game_win, row, col, "%lc", GRID_BR);
#endif
	wrefresh(game_win);

#if 0
	/* TODO Timer */
	if (game->state == RUNNING) {
		// start timer
	} else if (game->state == PAUSED) {
		game->state = RUNNING;
		// resume timer
	} else if (game->state == ABORTED) {
		game->state = NOT_RUNNING;
		// stop timer
	}
#endif

	/* New game */
	row_m = col_m = 0;
	row = col = 1;

	/* Highlight start field */
	wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
	field_val = game_get_value(game, row_m, col_m);
	gui_game_print_field(game_win, row, col, field_val);
	wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
	wrefresh(game_win);

	/* Handle player input */
	while (game->state == RUNNING) {
		input = getch();
		if (input == game->controls.up) {
			if (row_m == 0) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			--row_m;
			row -= 2;
			/* Hightlight the new field */
			wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			wrefresh(game_win);
		} else if (input == game->controls.down) {
			if (row_m == game->cfg.rows - 1) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			++row_m;
			row += 2;
			/* Hightlight the new field */
			wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			wrefresh(game_win);
		} else if (input == game->controls.left) {
			if (col_m == 0) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			--col_m;
			col -= 2;
			/* Hightlight the new field */
			wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			wrefresh(game_win);
		} else if (input == game->controls.right) {
			if (col_m == game->cfg.columns - 1) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			++col_m;
			col += 2;
			/* Hightlight the new field */
			wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			wrefresh(game_win);
		} else if (input == game->controls.reveal) {
			game_reveal(game, row, col);
			wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			wrefresh(game_win);
		} else if (input == game->controls.toggle_flag) {
			game_toggle_flag(game, row, col);
			wattron(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			field_val = game_get_value(game, row_m, col_m);
			gui_game_print_field(game_win, row, col, field_val);
			wattroff(game_win, COLOR_PAIR(COLOR_GAME_CURSOR));
			wrefresh(game_win);
		} else if (input == KEY_F(1)) {
			game->state = ABORTED;
		}
	}
	werase(game_win);
	wrefresh(game_win);
}

void
gui_options_show(struct game *game, struct gui *gui)
{
	WINDOW *opts_win;

	opts_win = gui->options_win;
	if (!opts_win) {
		opts_win = newwin(10, 20, offset_top, X_CENTER(COLS, 20));
	}

	mvwprintw(opts_win, 0, 0, "in options window");
	wrefresh(opts_win);
	getch();

	werase(opts_win);
	wrefresh(opts_win);
}

void
gui_destroy(struct gui *gui)
{
	int i;
	ITEM **items;

	if (gui->menu.menu) {
		unpost_menu(gui->menu.menu);
		items = menu_items(gui->menu.menu);
		for(i = 0; i < (item_count(gui->menu.menu) + 1); ++i) {
			free_item(items[i]);
		}
		free(items);
		free_menu(gui->menu.menu);
	}
	if (gui->menu.menu_win) {
		delwin(gui->menu.menu_win);
	}
	if (gui->game_win) {
		delwin(gui->game_win);
	}
	if (gui->options_win) {
		delwin(gui->options_win);
	}
	endwin();					/* End ncurses mode */
}

static void
menu_size(MENU *menu, int *height, int *width)
{
	int _width, mark_width;

	_width = menu->width - 2; /* -2 because apparently the default mark gets taken into account ??? */
	if (menu_mark(menu) != NULL) {
		mark_width = strlen(menu_mark(menu));
		_width += mark_width;
	}
	*height = menu->height;
	*width = _width;
}

static void
print_title(struct game *game, WINDOW *win, int row, int col)
{
	int i, col_start;

	col_start = col;
	for (i = 0; i < game->title.len; ++i, ++col) {
		if (game->title.title[i] == '\n') {
			++row;
			col = col_start - 1; // why -1 ????
		}
		mvwprintw(win, row, col, "%lc", game->title.title[i]);
	}
}

static void
gui_game_print_field(WINDOW *win, int row, int col, int field_val)
{
	if (field_val == FLAG_OFF) {
		field_val = FIELD_DEFAULT;
	} else if (field_val == FLAG_ON) {
		field_val = FIELD_FLAG;
	} else {
		field_val = L'0' + field_val;
	}
	mvwprintw(win, row, col, "%lc", field_val);
}
