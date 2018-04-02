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
#define DEFAULT L'\u2591'
#define FLAG L'\u2690'

/* Grid symbols */
#define GRID_T L'\u252c'
#define GRID_TL L'\u250c'
#define GRID_TR L'\u2510'
#define GRID_B L'\u2534'
#define GRID_BL L'\u2514'
#define GRID_BR L'\u2518'
#define GRID_L L'\u251c'
#define GRID_R L'\u2524'
#define GRID_X L'\u253c'
#define GRID_H L'\u2502'
#define GRID_V L'\u2500'

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

void
gui_init(void)
{
	initscr();					/* Init curses mode */
	noecho();					/* Dont show input data */
	curs_set(0);					/* Make cursor invisible */
	cbreak();					/* Line buffering disabled, pass every char on */
	start_color();					/* Enable colors */
	keypad(stdscr, TRUE);				/* Enable the keypad */

	/* Define color pairs */
	/* ID, forground, background */
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_BLACK, COLOR_GREEN);
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
		print_title(game, stdscr, title_margin_top, X_CENTER(COLS, title_width));
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
	uint32_t field_val;

	game_win = gui->game_win;
	if (!game_win) {
		/* account for border size */
		game_win = newwin(game_win_height, game_win_width, offset_top, X_CENTER(COLS, game_win_width));
	}


/* Grid symbols */
#define GRID_T L'\u252c'
#define GRID_TL L'\u250c'
#define GRID_TR L'\u2510'
#define GRID_B L'\u2534'
#define GRID_BL L'\u2514'
#define GRID_BR L'\u2518'
#define GRID_L L'\u251c'
#define GRID_R L'\u2524'
#define GRID_X L'\u253c'
#define GRID_H L'\u2502'
#define GRID_V L'\u2500'


	game_win_height = (game->cfg.rows * 2) + 1;
	game_win_width = (game->cfg.columns * 2) + 1;
	row_m = col_m = 0;
	/* Print top border */
	for (col = 0; col < game_win_width; ++col) {
		if (col == 0) {
			mvwprintw(game_win, row, col, "%lc", GRID_TL);
		} else if (col == (game_win_width - 1)) {
			mvwprintw(game_win, row, col, "%lc", GRID_TR);
		} else {
			mvwprintw(game_win, row, col, "%lc", GRID_T);
		}
	}

	/* Print bottom border */
	for (col = 0; col < game_win_width; ++col) {
		if (col == 0) {
			mvwprintw(game_win, row, col, "%lc", GRID_BL);
		} else if (col == (game_win_width - 1)) {
			mvwprintw(game_win, row, col, "%lc", GRID_BR);
		} else {
			mvwprintw(game_win, row, col, "%lc", GRID_B);
		}
	}
#if 0
	for (row = 0; row < game_win_height; ++row) {
		if (row == 0) {
			if (row == (game_win_height - 1) && col == 0) {
				mvwprintw(game_win, row, col, "%lc", GRID_BL);
				continue;
			}
			if (row == (game_win_height - 1) && col == (game_win_width - 1)) {
				mvwprintw(game_win, row, col, "%lc", GRID_BR);
				continue;
			}
		}
		if (row == (game_win_height - 1)) {
		}

		if (row % 2) {
			for (col = 0; col < game_win_width; ++col) {
				if (col % 2) {
					field_val = ('0' + matrix_get_value(game->surface, row_m, col_m++)) != '0' ? field_val : DEFAULT;
					mvwprintw(game_win, row, col, "%lc", field_val);
				} else {
					mvwprintw(game_win, row, col, "%lc", FIELD_SEP_H);
				}
			}
			++row_m;
		} else {
			for (col = 0; col < game_win_width; ++col) {
				if (col % 2) {
					mvwprintw(game_win, row, col, "%lc", FIELD_SEP_V);
				} else {
					mvwprintw(game_win, row, col, "%lc", FIELD_SEP_HV);
				}
			}
		}
	}
	wrefresh(game_win);
	getch();

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

	_width = menu->width - 2; /* -2 because apparently the default mark gets taken into account */
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
