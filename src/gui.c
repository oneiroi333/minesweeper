#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gui.h"
#include "utf8_lib.h"
#include "utils.h"
#include "colors.h"
#include "graphics.h"

/* Enter key code */
#ifdef KEY_ENTER
#undef KEY_ENTER
#endif
#define KEY_ENTER 0xA

/* Field symbols */
#define SYM_FLAG_OFF L'\u2591'
#define SYM_FLAG_ON L'\u2690'
#define SYM_MINE L'X'
#define SYM_EMPTY L' '

/* Grid symbols */
#define SYM_GRID_T L'\u252c'	/* top */
#define SYM_GRID_TL L'\u250c'	/* top-left */
#define SYM_GRID_TR L'\u2510'	/* top-right */
#define SYM_GRID_B L'\u2534'	/* bottom */
#define SYM_GRID_BL L'\u2514'	/* bottom-left */
#define SYM_GRID_BR L'\u2518'	/* bottom-right */
#define SYM_GRID_L L'\u251c'	/* left */
#define SYM_GRID_R L'\u2524'	/* right */
#define SYM_GRID_X L'\u253c'	/* intersection */
#define SYM_GRID_H L'\u2500'	/* vertical */
#define SYM_GRID_V L'\u2502'	/* horizontal */

#define CENTER(container, obj) \
	((container / 2) - (obj / 2))

#define COMPUTE_COLOR(val, color)		\
	do {					\
		if (val < 1) {			\
			color = COLOR_WHITE;	\
		} else if (val < 6) {		\
			color = val + 1;	\
		} else {			\
			color = val - 5;	\
		}				\
	} while (0)

#define COMPUTE_SYMBOL(val, symbol)		\
	do {					\
		if (val > 0) {			\
			symbol = L'0' + val;	\
		} else if (val == FLAG_OFF) {	\
			symbol = SYM_FLAG_OFF;	\
		} else if (val == FLAG_ON) {	\
			symbol = SYM_FLAG_ON;	\
		} else {			\
			symbol = SYM_EMPTY;	\
		}				\
	} while (0)

static void menu_size(MENU *menu, int *height, int *width);
static void print_title(struct game *game, WINDOW *win, int row, int col);
static void print_field(WINDOW *win, int row, int col, int color, uint32_t symbol);
static void gui_game_over_show(struct game *game, WINDOW *win);
static void print_game_with_grid(struct game *game, WINDOW *win);
static void print_game_without_grid(struct game *game, WINDOW *win);

// TODO
// this values should be set in game_init() in relation to the 'struct title'!!!
/* title */
int title_height = 10;
int title_width = 126;
int title_margin_top = 3;
int title_margin_bottom = 3;
int offset_top = 16;
int menu_bar_win_height = 1;
int menu_bar_win_width = 126;
int game_over_win_height = 20;
int game_over_win_width = 126;

void
gui_init(struct gui *gui)
{
	int i;

	initscr();					/* Init curses mode */
	noecho();					/* Dont show input data */
	curs_set(0);					/* Make cursor invisible */
	cbreak();					/* Line buffering disabled, pass every char on */
	start_color();					/* Enable colors */
	keypad(stdscr, TRUE);				/* Enable the keypad */
	/* Init color pairs */
	for (i = 0; i < 8; ++i) {
		init_pair(i, i, COLOR_BLACK);
	}
	memset(gui, 0, sizeof(struct gui));
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
		wattron(stdscr, COLOR_PAIR(COLOR_RED));
		print_title(game, stdscr, title_margin_top, CENTER(COLS, title_width));
		wattroff(stdscr, COLOR_PAIR(COLOR_RED));
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
		menu_x_start = CENTER(COLS, menu_width);
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
		if (input == KEY_UP) {
			menu_driver(menu, REQ_UP_ITEM);
			wrefresh(menu_win);
		} else if (input == KEY_DOWN) {
			menu_driver(menu, REQ_DOWN_ITEM);
			wrefresh(menu_win);
		} else if (input == KEY_ENTER) {
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
	WINDOW *game_win, *menu_bar_win, *game_over_win;
	int game_win_height, game_win_width;
	int row, col, row_m, col_m, field_val, grid_thickness;
	int input, color;
	uint32_t symbol;

	row_m = col_m = 0;
	if (game->cfg.grid == GRID_OFF) {
		game_win_height = game->cfg.rows;
		game_win_width = game->cfg.columns;
		row = col = 0;
		grid_thickness = 0;
	} else {
		game_win_height = (game->cfg.rows * 2) + 1;
		game_win_width = (game->cfg.columns * 2) + 1;
		row = col = 1;
		grid_thickness = 1;
	}

	game_win = gui->game_win;
	menu_bar_win = gui->menu_bar_win;
	game_over_win = gui->game_over_win;
	if (!game_win) {
		game_win = newwin(game_win_height, game_win_width, offset_top, CENTER(COLS, game_win_width));
	}
	if (!menu_bar_win) {
		menu_bar_win = newwin(menu_bar_win_height, menu_bar_win_width, offset_top - 2, CENTER(COLS, menu_bar_win_width));
	}
	if (!game_over_win) {
		game_over_win = newwin(game_over_win_height, game_over_win_width, offset_top, CENTER(COLS, game_over_win_width));
	}

	/* Print menu bar */
	mvwprintw(menu_bar_win, 0, 0, "%s", "F1: QUIT");
	wrefresh(menu_bar_win);

	/* Print game field */
	if (game->cfg.grid == GRID_ON) {
		print_game_with_grid(game, game_win);
	} else {
		print_game_without_grid(game, game_win);
	}

	/* Highlight start field */
	print_field(game_win, row, col, COLOR_RED, SYM_FLAG_OFF);
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
			COMPUTE_COLOR(field_val, color);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, color, symbol);
			--row_m;
			row -= (1 + grid_thickness);
			/* Hightlight the new field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, COLOR_RED, symbol);
			wrefresh(game_win);
		} else if (input == game->controls.down) {
			if (row_m == game->cfg.rows - 1) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_COLOR(field_val, color);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, color, symbol);
			++row_m;
			row += (1 + grid_thickness);
			/* Hightlight the new field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, COLOR_RED, symbol);
			wrefresh(game_win);
		} else if (input == game->controls.left) {
			if (col_m == 0) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_COLOR(field_val, color);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, color, symbol);
			--col_m;
			col -= (1 + grid_thickness);
			/* Hightlight the new field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, COLOR_RED, symbol);
			wrefresh(game_win);
		} else if (input == game->controls.right) {
			if (col_m == game->cfg.columns - 1) {
				continue;
			}
			/* Remove highlight of the previous field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_COLOR(field_val, color);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, color, symbol);
			++col_m;
			col += (1 + grid_thickness);
			/* Hightlight the new field */
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, COLOR_RED, symbol);
			wrefresh(game_win);
		} else if (input == game->controls.reveal) {
			game_reveal(game, row_m, col_m);
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_COLOR(field_val, color);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, color, symbol);
			wrefresh(game_win);
		} else if (input == game->controls.toggle_flag) {
			game_toggle_flag(game, row_m, col_m);
			field_val = game_get_value(game, row_m, col_m);
			COMPUTE_COLOR(field_val, color);
			COMPUTE_SYMBOL(field_val, symbol);
			print_field(game_win, row, col, color, symbol);
			wrefresh(game_win);
		} else if (input == KEY_F(1)) {
			game->state = ABORTED;
		}
	}
	werase(game_win);
	wrefresh(game_win);
	werase(menu_bar_win);
	wrefresh(menu_bar_win);

	gui_game_over_show(game, game_over_win);
}

void
gui_options_show(struct game *game, struct gui *gui)
{
	WINDOW *opts_win;

	opts_win = gui->options_win;
	if (!opts_win) {
		opts_win = newwin(10, 20, offset_top, CENTER(COLS, 20));
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
	if (gui->menu_bar_win) {
		delwin(gui->menu_bar_win);
	}
	if (gui->game_over_win) {
		delwin(gui->game_over_win);
	}
	if (gui->options_win) {
		delwin(gui->options_win);
	}
	endwin();					/* End ncurses mode */
}

static void
gui_game_over_show(struct game *game, WINDOW *win)
{
	char *skull;
	int len, i, col, col_start, row;
	
	skull = read_file(PATH_SKULL);
	len = strlen(skull);
	col = col_start = row = 0;
	if (is_valid_utf8(skull, len)) {
		for (i = 0; i < len; ++i, ++col) {
			if (skull[i] == '\n') {
				++row;
				col = col_start - 1; // why -1 ????
			}
			mvwprintw(win, row, col, "%lc", skull[i]);
		}
		if (game->state == GAME_OVER && game->outcome == VICTORY) {
			mvwprintw(win, 15, 60, "%s", "YOU DID ESCAPE THIS TIME!");
		} else if (game->state == GAME_OVER && game->outcome == DEFEAT) {
			mvwprintw(win, 15, 60, "%s", "YOU DIED A HORRIBLE DEATH!");
		}
		wrefresh(win);
	}
	getch();
	werase(win);
	wrefresh(win);
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
print_field(WINDOW *win, int row, int col, int color, uint32_t symbol)
{
	wattron(win, COLOR_PAIR(color));
	mvwprintw(win, row, col, "%lc", symbol);
	wattroff(win, COLOR_PAIR(color));
}

static void
print_game_with_grid(struct game *game, WINDOW *win)
{
	int row, col, row_m, col_m, win_height, win_width;

	win_height = (game->cfg.rows * 2) + 1;
	win_width = (game->cfg.columns * 2) + 1;
	row = col = 0;
	row_m = col_m = 0;
	/* Print top border */
	print_field(win, row, col, game->cfg.grid_color, SYM_GRID_TL);
	//mvwprintw(win, row, col, "%lc", SYM_GRID_TL);
	for (col = 1; col < win_width - 1; ++col) {
		if (col % 2) {
			print_field(win, row, col, game->cfg.grid_color, SYM_GRID_H);
			//mvwprintw(win, row, col, "%lc", SYM_GRID_H);
		} else {
			print_field(win, row, col, game->cfg.grid_color, SYM_GRID_T);
			//mvwprintw(win, row, col, "%lc", SYM_GRID_T);
		}
	}
	mvwprintw(win, row++, col, "%lc", SYM_GRID_TR);
	/* Print the middle part */
	for (; row < win_height - 1; ++row) {
		if (row % 2) {
			for (col = 0; col < win_width; ++col) {
				if (col % 2) {
					print_field(win, row, col, COLOR_WHITE, SYM_FLAG_OFF);
					++col_m;
				} else {
					print_field(win, row, col, game->cfg.grid_color, SYM_GRID_V);
					//mvwprintw(win, row, col, "%lc", SYM_GRID_V);
				}
			}
			col_m = 0;
			++row_m;
		} else {
			for (col = 0; col < win_width; ++col) {
				if (col == 0) {
					print_field(win, row, col, game->cfg.grid_color, SYM_GRID_L);
					//mvwprintw(win, row, col, "%lc", SYM_GRID_L);
				} else if (col == win_width - 1) {
					print_field(win, row, col, game->cfg.grid_color, SYM_GRID_R);
					//mvwprintw(win, row, col, "%lc", SYM_GRID_R);
				} else if (col % 2) {
					print_field(win, row, col, game->cfg.grid_color, SYM_GRID_H);
					//mvwprintw(win, row, col, "%lc", SYM_GRID_H);
				} else {
					print_field(win, row, col, game->cfg.grid_color, SYM_GRID_X);
					//mvwprintw(win, row, col, "%lc", SYM_GRID_X);
				}
			}
		}
	}
	/* Print bottom border */
	col = 0;
	print_field(win, row, col, game->cfg.grid_color, SYM_GRID_BL);
	//mvwprintw(win, row, col, "%lc", SYM_GRID_BL);
	for (col = 1; col < win_width - 1; ++col) {
		if (col % 2) {
			print_field(win, row, col, game->cfg.grid_color, SYM_GRID_H);
			//mvwprintw(win, row, col, "%lc", SYM_GRID_H);
		} else {
			print_field(win, row, col, game->cfg.grid_color, SYM_GRID_B);
			//mvwprintw(win, row, col, "%lc", SYM_GRID_B);
		}
	}
	print_field(win, row, col, game->cfg.grid_color, SYM_GRID_BR);
	//mvwprintw(win, row, col, "%lc", SYM_GRID_BR);

	wrefresh(win);
}

static void
print_game_without_grid(struct game *game, WINDOW *win)
{
	int row, col;

	for (row = 0; row < game->cfg.rows; ++row) {
		for (col = 0; col < game->cfg.columns; ++col) {
			print_field(win, row, col, COLOR_WHITE, SYM_FLAG_OFF);
		}
	}
}
