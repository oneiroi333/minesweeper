#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>
#include "gui.h"
#include "utf8_lib.h"
#include "colors.h"
#include "graphics.h"
#include "symbols.h"
#include "../core/game.h"
#include "../core/utils.h"

#define DEBUG

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

#define COMPUTE_SYMBOL(val, symbol)			\
	do {						\
		if (val > 0) {				\
			symbol = L'0' + val;		\
		} else if (val == FIELD_FLAG_OFF) {	\
			symbol = SYM_FLAG_OFF;		\
		} else if (val == FIELD_FLAG_ON) {	\
			symbol = SYM_FLAG_ON;		\
		} else {				\
			symbol = SYM_EMPTY;		\
		}					\
	} while (0)

extern struct game *game_p;

#ifdef DEBUG
WINDOW *dbg_win;
#endif

static int gui_menu_show(struct gui *gui);
static void gui_game_show(struct gui *gui);
static void gui_options_show(struct gui *gui);
static void gui_game_over_show(struct gui *gui, int outcome);

static void gui_handle_player_input(struct gui *gui, struct difficulty *difficulty, struct controls *controls, int input);
static void gui_menu_size_get(MENU *menu, int *width, int *height);
static void gui_print_graphic(char *data, WINDOW *win, int start_pos_y, int start_pos_x);
static void gui_print_char(WINDOW *win, int row, int col, int color, char character);
static void gui_print_ucs4_graphic(uint32_t *data, int data_len, WINDOW *win, int start_pos_y, int start_pos_x);
static void gui_print_ucs4_char(WINDOW *win, int row, int col, int color, uint32_t character);

static void gui_print_minefield_grid_on(struct matrix *minefield, WINDOW *win);
static void gui_print_minefield_grid_off(struct matrix *minefield, WINDOW *win);

void
gui_init(struct gui *gui)
{
	WINDOW *menu_win, *menu_sub_win;
	MENU *menu;
	ITEM **items;
	int i, data_len;
	size_t ucs4_len;
	char *data_raw;
	struct difficulty *diff;

	setlocale(LC_ALL, "");

	/* Ncurses stuff */
	initscr();					/* Init curses mode */
	refresh();					/* have to call it idk why */
	noecho();					/* Dont echo input data */
	curs_set(0);					/* Make cursor invisible */
	cbreak();					/* Line buffering disabled, pass every char on */
	start_color();					/* Enable colors */
	keypad(stdscr, TRUE);				/* Enable the keypad */
	for (i = 0; i < COLOR_COUNT; ++i) {		/* Init color pairs (background: black) */
		init_pair(i, i, COLOR_BLACK);
	}

#ifdef DEBUG
	dbg_win = newwin(10, 20, 0, 0);
	box(dbg_win, '|', '-');
#endif

	/* Init skull */
	gui->skull.skull_win.width = 44;		/* skull max line length */
	gui->skull.skull_win.height = 20;		/* skull line count */

	/* Init title */
	gui->title.title_win.width = 126;		/* title max line length */
	gui->title.title_win.height = 10;		/* title line count */
	gui->title.title_win.pos_y = CENTER(LINES, (gui->title.title_win.height + 3 + gui->skull.skull_win.height));
	gui->title.title_win.pos_x = CENTER(COLS, gui->title.title_win.width);
	gui->title.title_win.win = newwin(gui->title.title_win.height, gui->title.title_win.width, gui->title.title_win.pos_y, gui->title.title_win.pos_x);

	data_raw = read_file(PATH_TITLE);
	data_len = strlen(data_raw);
	if (is_valid_utf8(data_raw, data_len)) {
		gui->title.data = utf8_to_ucs4(data_raw, data_len, &ucs4_len);
		gui->title.data_len = ucs4_len;
	} else {
		gui->title.data = NULL;
		gui->title.data_len = 0;
	}
	free(data_raw);

	/* Init skull */
	gui->skull.skull_win.pos_y = CENTER(LINES, (gui->title.title_win.height + gui->skull.skull_win.height)) + gui->title.title_win.height;
	gui->skull.skull_win.pos_x = gui->title.title_win.pos_x;
	gui->skull.skull_win.win = newwin(gui->skull.skull_win.height, gui->skull.skull_win.width, gui->skull.skull_win.pos_y, gui->skull.skull_win.pos_x);

	gui->skull.data = read_file(PATH_SKULL);

	/* Init menu */
	items = (ITEM **) malloc(4 * sizeof(ITEM *));
	items[0] = new_item("P L A Y", "");
	items[1] = new_item("O P T I O N S", "");
	items[2] = new_item("Q U I T", "");
	items[3] = (ITEM *) NULL;
	menu = new_menu(items);
	gui_menu_size_get(menu, &gui->menu.menu_win.width, &gui->menu.menu_win.height);

	gui->menu.menu_win.pos_y = gui->title.title_win.pos_y + gui->title.title_win.height + CENTER(gui->skull.skull_win.height, gui->menu.menu_win.height);
	gui->menu.menu_win.pos_x = CENTER(COLS, gui->menu.menu_win.width);

	menu_win = newwin(gui->menu.menu_win.height, gui->menu.menu_win.width, gui->menu.menu_win.pos_y, gui->menu.menu_win.pos_x);
	menu_sub_win = derwin(menu_win, gui->menu.menu_win.height, gui->menu.menu_win.width, 0, 0);
	set_menu_win(menu, menu_win);
	set_menu_sub(menu, menu_sub_win);
	set_menu_mark(menu, "");

	gui->menu.menu_win.win = menu_win;
	gui->menu.menu_sub_win.win = menu_sub_win;
	gui->menu.menu = menu;

	/* Init options */
	gui->options.options_win.win = newwin(10, 20, 16, CENTER(COLS, 20));
	gui->options.grid = OPT_GRID_OFF;

	/* Init game */
	/* Get the width/height of the minefield */
	diff = game_config_difficulty_get(game_p);
	/* If the grid is set we need twice as much width/height */
	gui->game.game_play_win.height = gui->options.grid == OPT_GRID_ON ? diff->lvl_rows[diff->lvl] * 2 + 1 : diff->lvl_rows[diff->lvl];
	gui->game.game_play_win.width = gui->options.grid == OPT_GRID_ON ? diff->lvl_columns[diff->lvl] * 2 + 1 : diff->lvl_columns[diff->lvl];
	gui->game.game_play_win.pos_y = CENTER(LINES, (gui->title.title_win.height + gui->game.game_play_win.height)) + gui->title.title_win.height;
	gui->game.game_play_win.pos_x = CENTER(COLS, gui->game.game_play_win.width);
	/* set window size before game start depending on the size of the minefield (currently done) OR set a max width and height for the minefield and init the window here with max size */
	gui->game.game_play_win.win = newwin(gui->game.game_play_win.height, gui->game.game_play_win.width, gui->game.game_play_win.pos_y, gui->game.game_play_win.pos_x);

	gui->game.game_over_win.height = 5;
	gui->game.game_over_win.width = 20;
	gui->game.game_over_win.pos_y = CENTER(LINES, 5);
	gui->game.game_over_win.pos_x = CENTER(COLS, 20);
	gui->game.game_over_win.win = newwin(gui->game.game_over_win.height, gui->game.game_over_win.width, gui->game.game_over_win.pos_y, gui->game.game_over_win.pos_x);

	// TODO
	gui->game.game_menu_bar_win.win = NULL;
}

void
gui_run(struct gui *gui)
{
	int choice, first_round;

	first_round = 1;
	do {
		choice = gui_menu_show(gui);
		switch (choice) {
			case OPT_PLAY:
				if (!first_round) {
					game_reinit(game_p);
					first_round = 0;
				}
				game_start(game_p);
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
	if (gui->menu.menu_sub_win.win) {
		delwin(gui->menu.menu_sub_win.win);
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

static int
gui_menu_show(struct gui *gui)
{
	struct controls *controls;
	int input;

	controls = game_config_controls_get(game_p);

	/* Print title */
	wattron(gui->title.title_win.win, COLOR_PAIR(COLOR_RED));
	gui_print_ucs4_graphic(gui->title.data, gui->title.data_len, gui->title.title_win.win, 0, 0);
	wattroff(gui->title.title_win.win, COLOR_PAIR(COLOR_RED));
	wrefresh(gui->title.title_win.win);

	/* Print skull */
	gui_print_graphic(gui->skull.data, gui->skull.skull_win.win, 0, 0);
	wrefresh(gui->skull.skull_win.win);

	/* Print menu*/
	post_menu(gui->menu.menu);
	wrefresh(gui->menu.menu_sub_win.win);
	for(;;) {
		input = getch();
		if (input == controls->up) {
			menu_driver(gui->menu.menu, REQ_UP_ITEM);
			wrefresh(gui->menu.menu_sub_win.win);
		} else if (input == controls->down) {
			menu_driver(gui->menu.menu, REQ_DOWN_ITEM);
			wrefresh(gui->menu.menu_sub_win.win);
		} else if (input == controls->down) {
			menu_driver(gui->menu.menu, REQ_DOWN_ITEM);
			wrefresh(gui->menu.menu_sub_win.win);
		} else if (input == controls->reveal) {
			/* Clear menu */
			unpost_menu(gui->menu.menu);
			wrefresh(gui->menu.menu_sub_win.win);
			wclear(gui->menu.menu_win.win);
			wrefresh(gui->menu.menu_win.win);
			/* Clear skull */
			wclear(gui->skull.skull_win.win);
			wrefresh(gui->skull.skull_win.win);
			return item_index(current_item(gui->menu.menu));
		}
	}
}

void
gui_game_show(struct gui *gui)
{
	struct game_state *game_state;
	struct controls *controls;
	struct difficulty *difficulty;
	int input;

	/* Print menu bar */
	//mvwprintw(gui->game.game_menu_bar_win.win, 0, 0, "%s", "F1: MENU");
	//wrefresh(menu_bar_win);

	/* Print game field */
	if (gui->options.grid == OPT_GRID_ON) {
		gui_print_minefield_grid_on(game_p->playground.surface, gui->game.game_play_win.win);
	} else {
		gui_print_minefield_grid_off(game_p->playground.surface, gui->game.game_play_win.win);
	}

	game_state = game_state_get(game_p);
	controls = game_config_controls_get(game_p);
	difficulty = game_config_difficulty_get(game_p);

	/* Highlight start field */
	gui_print_ucs4_char(gui->game.game_play_win.win, gui->options.grid == OPT_GRID_ON ? 1 : 0, gui->options.grid == OPT_GRID_ON ? 1 : 0, COLOR_RED, SYM_FLAG_OFF);
	wrefresh(gui->game.game_play_win.win);

	/*
	 * PLAY LOOP
	 */
	do {
#ifdef DEBUG
		mvwprintw(dbg_win, 1, 1, "input: %d", input);
		mvwprintw(dbg_win, 2, 1, "state: %d", game_state->state);
		mvwprintw(dbg_win, 3, 1, "pos_y: %2d", game_playground_get_pos_y_player(game_p));
		mvwprintw(dbg_win, 4, 1, "pos_x: %2d", game_playground_get_pos_x_player(game_p));
		wrefresh(dbg_win);
#endif
		input = getch();
		gui_handle_player_input(gui, difficulty, controls, input);
	} while (game_state->state == GAME_RUNNING);
#ifdef DEBUG
		mvwprintw(dbg_win, 1, 1, "input: %d", input);
		mvwprintw(dbg_win, 2, 1, "state: %d", game_state->state);
		mvwprintw(dbg_win, 3, 1, "pos_y: %2d", game_playground_get_pos_y_player(game_p));
		mvwprintw(dbg_win, 4, 1, "pos_x: %2d", game_playground_get_pos_x_player(game_p));
		wrefresh(dbg_win);
#endif

	if (game_state->state == GAME_OVER) {
		gui_game_over_show(gui, game_state->outcome);
	} else if (game_state->state == GAME_ABORTED) {
		/* Player left the game before it was over */
		// display something on the menu window? like: run coward run! :)
	}
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
gui_game_over_show(struct gui *gui, int outcome)
{
	WINDOW * win;

	win = gui->game.game_over_win.win;

	if (outcome == OUTCOME_DEFEAT) {
		wattron(win, COLOR_PAIR(COLOR_GREEN));
		mvwprintw(win, 15, 60, "%s", MSG_VICTORY);
		wattroff(win, COLOR_PAIR(COLOR_GREEN));
	} else {
		wattron(win, COLOR_PAIR(COLOR_RED));
		mvwprintw(win, 15, 60, "%s", MSG_DEFEAT);
		wattroff(win, COLOR_PAIR(COLOR_RED));
	}
	wrefresh(win);
	getch();
	wclear(win);
	wrefresh(win);
}

static void
gui_handle_player_input(struct gui *gui, struct difficulty *difficulty, struct controls *controls, int input)
{
	WINDOW *game_play_win;
	int pos_y_player, pos_x_player;
	int field_val, color;
	uint32_t symbol;

	game_play_win = gui->game.game_play_win.win;
	pos_y_player = game_playground_get_pos_y_player(game_p);
	pos_x_player = game_playground_get_pos_x_player(game_p);

	/* Remove highlight of the old field */
	field_val = game_playground_get(game_p, pos_y_player, pos_x_player);
	COMPUTE_COLOR(field_val, color);
	COMPUTE_SYMBOL(field_val, symbol);
	gui_print_ucs4_char(game_play_win, pos_y_player, pos_x_player, color, symbol);
	wrefresh(game_play_win);

	/* Compute new pos */
	if (input == controls->up) {
		if (pos_y_player > 0) {
			--pos_y_player;
		}
	} else if (input == controls->down) {
		if (pos_y_player < difficulty->lvl_rows[difficulty->lvl] - 1) {
			++pos_y_player;
		}
	} else if (input == controls->left) {
		if (pos_x_player > 0) {
			--pos_x_player;
		}
	} else if (input == controls->right) {
		if (pos_x_player < difficulty->lvl_columns[difficulty->lvl] - 1) {
			++pos_x_player;
		}
	} else if (input == controls->reveal) {
		field_val = game_playground_reveal(game_p, pos_y_player, pos_x_player);
		goto highlight;
	} else if (input == controls->toggle_flag) {
		game_playground_toggle_flag(game_p, pos_y_player, pos_x_player);
	} else if (input == KEY_F(1)) {
		game_quit(game_p);
	}

	game_playground_set_pos_y_player(game_p, pos_y_player);
	game_playground_set_pos_x_player(game_p, pos_x_player);

	/* Hightlight the new field */
	field_val = game_playground_get(game_p, pos_y_player, pos_x_player);
highlight:
	COMPUTE_SYMBOL(field_val, symbol);
	if (gui->options.grid == OPT_GRID_OFF) {
		gui_print_ucs4_char(game_play_win, pos_y_player, pos_x_player, COLOR_RED, symbol);
	} else {
		gui_print_ucs4_char(game_play_win, pos_y_player * 2, pos_x_player * 2, COLOR_RED, symbol);
	}
	wrefresh(game_play_win);
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
gui_print_graphic(char *data, WINDOW *win, int start_pos_y, int start_pos_x)
{
	int i;
	int pos_x, pos_y;
	char *dp;

	pos_y = start_pos_y;
	pos_x = start_pos_x;
	dp = data;
	while (*dp) {
		if (*dp == '\n') {
			++pos_y;		/* move to next line */
			pos_x = start_pos_x;	/* carriage return */
			++dp;			/* skip newline */
		}
		mvwaddch(win, pos_y, pos_x++, *dp);
		++dp;
	}

}

static void
gui_print_char(WINDOW *win, int row, int col, int color, char character)
{
	wattron(win, COLOR_PAIR(color));
	mvwprintw(win, row, col, "%c", character);
	wattroff(win, COLOR_PAIR(color));
}

static void
gui_print_ucs4_graphic(uint32_t *data, int data_len, WINDOW *win, int start_pos_y, int start_pos_x)
{
	int i;
	int pos_x, pos_y;

	pos_y = start_pos_y;
	pos_x = start_pos_x;
	for (i = 0; i < data_len; ++i, ++pos_x) {
		if (data[i] == '\n') {
			++pos_y;
			pos_x = start_pos_x - 1;
		}
		mvwprintw(win, pos_y, pos_x, "%lc", data[i]);
	}
}

static void
gui_print_ucs4_char(WINDOW *win, int row, int col, int color, uint32_t character)
{
	wattron(win, COLOR_PAIR(color));
	mvwprintw(win, row, col, "%lc", character);
	wattroff(win, COLOR_PAIR(color));
}

static void
gui_print_minefield_grid_on(struct matrix *minefield, WINDOW *win)
{
	int row, col, row_m, col_m, win_height, win_width;
	struct difficulty *diff;

	diff = game_config_difficulty_get(game_p);
	win_height = (diff->lvl_rows[diff->lvl] * 2) + 1;
	win_width = (diff->lvl_columns[diff->lvl] * 2) + 1;
	row = col = 0;
	row_m = col_m = 0;
	/* Print top border */
	//print_field(win, row, col, COLOR_WHITE, SYM_GRID_TL);
	mvwprintw(win, row, col, "%lc", SYM_GRID_TL);
	for (col = 1; col < win_width - 1; ++col) {
		if (col % 2) {
			//print_field(win, row, col, COLOR_WHITE, SYM_GRID_H);
			mvwprintw(win, row, col, "%lc", SYM_GRID_H);
		} else {
			//print_field(win, row, col, COLOR_WHITE, SYM_GRID_T);
			mvwprintw(win, row, col, "%lc", SYM_GRID_T);
		}
	}
	mvwprintw(win, row++, col, "%lc", SYM_GRID_TR);
	/* Print the middle part */
	for (; row < win_height - 1; ++row) {
		if (row % 2) {
			for (col = 0; col < win_width; ++col) {
				if (col % 2) {
					//print_field(win, row, col, COLOR_WHITE, SYM_FLAG_OFF);
					mvwprintw(win, row, col, "%lc", SYM_FLAG_OFF);
					++col_m;
				} else {
					//print_field(win, row, col, COLOR_WHITE, SYM_GRID_V);
					mvwprintw(win, row, col, "%lc", SYM_GRID_V);
				}
			}
			col_m = 0;
			++row_m;
		} else {
			for (col = 0; col < win_width; ++col) {
				if (col == 0) {
					//print_field(win, row, col, COLOR_WHITE, SYM_GRID_L);
					mvwprintw(win, row, col, "%lc", SYM_GRID_L);
				} else if (col == win_width - 1) {
					//print_field(win, row, col, COLOR_WHITE, SYM_GRID_R);
					mvwprintw(win, row, col, "%lc", SYM_GRID_R);
				} else if (col % 2) {
					//print_field(win, row, col, COLOR_WHITE, SYM_GRID_H);
					mvwprintw(win, row, col, "%lc", SYM_GRID_H);
				} else {
					//print_field(win, row, col, COLOR_WHITE, SYM_GRID_X);
					mvwprintw(win, row, col, "%lc", SYM_GRID_X);
				}
			}
		}
	}
	/* Print bottom border */
	col = 0;
	//print_field(win, row, col, COLOR_WHITE, SYM_GRID_BL);
	mvwprintw(win, row, col, "%lc", SYM_GRID_BL);
	for (col = 1; col < win_width - 1; ++col) {
		if (col % 2) {
			//print_field(win, row, col, COLOR_WHITE, SYM_GRID_H);
			mvwprintw(win, row, col, "%lc", SYM_GRID_H);
		} else {
			//print_field(win, row, col, COLOR_WHITE, SYM_GRID_B);
			mvwprintw(win, row, col, "%lc", SYM_GRID_B);
		}
	}
	//print_field(win, row, col, COLOR_WHITE, SYM_GRID_BR);
	mvwprintw(win, row, col, "%lc", SYM_GRID_BR);
	wrefresh(win);
}

static void
gui_print_minefield_grid_off(struct matrix *minefield, WINDOW *win)
{
	int row, col;

	for (row = 0; row < minefield->rows; ++row) {
		for (col = 0; col < minefield->columns; ++col) {
			gui_print_ucs4_char(win, row, col, COLOR_WHITE, SYM_FLAG_OFF);
		}
	}
	wrefresh(win);
}
