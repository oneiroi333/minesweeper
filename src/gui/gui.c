#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gui.h"
#include "utf8_lib.h"
#include "colors.h"
#include "graphics.h"
#include "symbols.h"
#include "../core/game.h"
#include "../core/utils.h"

extern struct game *game_p;

#ifdef KEY_ENTER
#undef KEY_ENTER
#endif
#define KEY_ENTER 0xA

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
		} else if (val == FIELD_FLAG_OFF) {	\
			symbol = SYM_FLAG_OFF;	\
		} else if (val == FIELD_FLAG_ON) {	\
			symbol = SYM_FLAG_ON;	\
		} else {			\
			symbol = SYM_EMPTY;	\
		}				\
	} while (0)

int gui_menu_show(struct gui *gui);
static void gui_game_show(struct gui *gui);
static void gui_options_show(struct gui *gui);

static void gui_menu_size_get(MENU *menu, int *width, int *height);
static void gui_print_title(struct gui *gui, WINDOW *win, int start_pos_y, int start_pos_x);

/*
static void menu_size(MENU *menu, int *height, int *width);
static void print_field(WINDOW *win, int row, int col, int color, uint32_t symbol);
static void gui_game_over_show(struct game *game, WINDOW *win);
static void print_game_with_grid(struct game *game, WINDOW *win);
static void print_game_without_grid(struct game *game, WINDOW *win);
*/

void
gui_init(struct gui *gui)
{
	WINDOW *menu_win, *menu_sub_win;
	MENU *menu;
	ITEM **items;
	int i, title_len;
	size_t ucs4_len;
	char *title;

	/* Ncurses stuff */
	initscr();					/* Init curses mode */
	noecho();					/* Dont echo input data */
	curs_set(0);					/* Make cursor invisible */
	cbreak();					/* Line buffering disabled, pass every char on */
	start_color();					/* Enable colors */
	keypad(stdscr, TRUE);				/* Enable the keypad */
	for (i = 0; i < COLOR_COUNT; ++i) {		/* Init color pairs (background: black) */
		init_pair(i, i, COLOR_BLACK);
	}

	/* Init title */
	gui->title.title_win.width = 126;		/* title max line length */
	gui->title.title_win.height = 10;		/* title line count */
	gui->title.title_win.pos_y = 3;
	gui->title.title_win.pos_x = CENTER(COLS, gui->title.title_win.width);
	gui->title.title_win.win = newwin(gui->title.title_win.height, gui->title.title_win.width, gui->title.title_win.pos_y, gui->title.title_win.pos_x);
	title = read_file(PATH_TITLE);
	title_len = strlen(title);
	if (is_valid_utf8(title, title_len)) {
		gui->title.data = utf8_to_ucs4(title, title_len, &ucs4_len);
		gui->title.data_len = ucs4_len;
	} else {
		gui->title.data = NULL;
		gui->title.data_len = 0;
	}
	free(title);

	/* Init menu */
	items = (ITEM **) malloc(4 * sizeof(ITEM *));
	items[0] = new_item("PLAY", "");
	items[1] = new_item("OPTIONS", "");
	items[2] = new_item("QUIT", "");
	items[3] = (ITEM *) NULL;
	menu = new_menu(items);
	set_menu_spacing(menu, 0, 2, 0);		/* Line spacing */
	gui_menu_size_get(menu, &gui->menu.menu_win.width, &gui->menu.menu_win.height);

	gui->menu.menu_win.pos_y = 16;
	gui->menu.menu_win.pos_x = CENTER(COLS, gui->menu.menu_win.width);

	menu_win = newwin(gui->menu.menu_win.height, gui->menu.menu_win.width, gui->menu.menu_win.pos_y, gui->menu.menu_win.pos_x);
	menu_sub_win = derwin(menu_win, gui->menu.menu_win.height, gui->menu.menu_win.width, 0, 0);
	set_menu_win(menu, menu_win);
	set_menu_sub(menu, menu_sub_win);
	set_menu_mark(menu, "");
	post_menu(menu);

	gui->menu.menu_win.win = menu_win;
	gui->menu.menu = menu;

	/* Init game */
	gui->game.game_play_win.win = NULL; /* set window size before game start depending on the size of the minefield (currently done) OR set a max width and height for the minefield and init the window here with max size */
	gui->game.game_over_win.win = NULL;
	gui->game.game_menu_bar_win.win = NULL;

	/* Init options */
	gui->options.options_win.win = newwin(10, 20, 16, CENTER(COLS, 20));
}

void
gui_run(struct gui *gui)
{
	int choice;

	do {
		choice = gui_menu_show(gui);
		switch (choice) {
			case OPT_PLAY:
				gui_game_show(gui);
				break;
			case OPT_OPTIONS:
				gui_options_show(gui);
				break;
			default:
				;
		}
	} while (choice != OPT_QUIT);
}

void
gui_destroy(struct gui *gui)
{
	ITEM **items;
	int i;

	/* Delete title */
	if (gui->title.title_win.win) {
		delwin(gui->title.title_win.win);
	}
	free(gui->title.data);

	/* Delete menu */
	if (gui->menu.menu) {
		unpost_menu(gui->menu.menu);
		items = menu_items(gui->menu.menu);
		for(i = 0; i < (item_count(gui->menu.menu) + 1); ++i) {
			free_item(items[i]);
		}
		free(items);
		free_menu(gui->menu.menu);
	}
	/* Delete menu win */
	if (gui->menu.menu_win.win) {
		delwin(gui->menu.menu_win.win);
	}

	/* Delete game win */
	if (gui->game.game_play_win.win) {
		delwin(gui->game.game_play_win.win);
	}
	if (gui->game.game_over_win.win) {
		delwin(gui->game.game_over_win.win);
	}
	if (gui->game.game_menu_bar_win.win) {
		delwin(gui->game.game_menu_bar_win.win);
	}

	/* Delete options win */
	if (gui->options.options_win.win) {
		delwin(gui->options.options_win.win);
	}

	/* End ncurses mode */
	endwin();
}

int
gui_menu_show(struct gui *gui)
{
	int input;
	struct controls *controls;

	controls = game_config_controls_get(game_p);

	/* Print title */
	wattron(gui->title.title_win.win, COLOR_PAIR(COLOR_RED));
	gui_print_title(gui, gui->title.title_win.win, 0, 0);
	wattroff(gui->title.title_win.win, COLOR_PAIR(COLOR_RED));
	wrefresh(gui->title.title_win.win);

	/* Print menu*/
	wrefresh(gui->menu.menu_win.win);
	for(;;) {
		input = getch();
		if (input == controls->up) {
			menu_driver(gui->menu.menu, REQ_UP_ITEM);
			wrefresh(gui->menu.menu_win.win);
		} else if (input == controls->down) {
			menu_driver(gui->menu.menu, REQ_DOWN_ITEM);
			wrefresh(gui->menu.menu_win.win);
		} else if (input == controls->reveal) {
			werase(gui->menu.menu_win.win);
			wrefresh(gui->menu.menu_win.win);
			return item_index(current_item(gui->menu.menu));
		}
	}
}

void
gui_game_show(struct gui *gui)
{
#if 0
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
#endif
}

void
gui_options_show(struct gui *gui)
{
	mvwprintw(gui->options.options_win.win, 0, 0, "in options window");
	wrefresh(gui->options.options_win.win);

	getch();

	werase(gui->options.options_win.win);
	wrefresh(gui->options.options_win.win);
}

static void
gui_game_over_show(struct game *game, WINDOW *win)
{
#if 0
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
#endif
}

static void
gui_menu_size_get(MENU *menu, int *width, int *height)
{
	int _width, mark_width;

	_width = menu->width - 2; /* -2 because apparently the default mark gets taken into account(?) */
	if (menu_mark(menu) != NULL) {
		mark_width = strlen(menu_mark(menu));
		_width += mark_width;
	}
	*height = menu->height;
	*width = _width;
}

static void
gui_print_title(struct gui *gui, WINDOW *win, int start_pos_y, int start_pos_x)
{
	int i;
	int pos_x, pos_y;

	pos_y = start_pos_y;
	pos_x = start_pos_x;
	for (i = 0; i < gui->title.data_len; ++i, ++pos_x) {
		if (gui->title.data[i] == '\n') {
			++pos_y;
			pos_x = start_pos_x; /* -1 */
		}
		mvwprintw(win, pos_x, pos_y, "%lc", gui->title.data[i]);
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
#if 0
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
#endif
}

static void
print_game_without_grid(struct game *game, WINDOW *win)
{
#if 0
	int row, col;

	for (row = 0; row < game->cfg.rows; ++row) {
		for (col = 0; col < game->cfg.columns; ++col) {
			print_field(win, row, col, COLOR_WHITE, SYM_FLAG_OFF);
		}
	}
#endif
}


