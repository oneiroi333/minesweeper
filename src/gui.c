#include <stdlib.h>
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

// TODO
// this values should be set in game_init() as part of the 'struct title'!!!
/* title */
int title_height = 10;
int title_width = 126;

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
gui_main_scr(struct game *game)
{
	WINDOW *menu_win, *menu_sub_win;
	MENU *menu;
	ITEM **items;
	int menu_height, menu_width, menu_items_count;
	int input, choice, i;
	//char *mark = "\u25BA ";
	char *mark = "";

#if 0
	/* Create the menu */
	menu_items_count = 3;				/* PLAY, OPTIONS, QUIT */
	items = (ITEM **) malloc((menu_items_count + 1) * sizeof(ITEM *));
	items[0] = new_item("PLAY", "");
	items[1] = new_item("OPTIONS", "");
	items[2] = new_item("QUIT", "");
	items[3] = (ITEM *) NULL;
	menu = new_menu(items);
	set_menu_spacing(menu, 0, 2, 0);		/* Line spacing */
	menu_size(menu, &menu_height, &menu_width);
#endif
	menu_win = newwin(0, 0, 0, 0);
#if 0
	menu_sub_win = derwin(menu_win, menu_height, menu_width, title_height + 10, (COLS / 2) - (menu_width / 2));
	set_menu_win(menu, menu_win);
	set_menu_sub(menu, menu_sub_win);
	set_menu_mark(menu, mark);
	post_menu(menu);
#endif

	/* Title */
	print_title(game, menu_win, 5, (COLS / 2) - (title_width / 2));

	/* Border */
	box(menu_win, '|', '-');
	//wborder(menu_win, '+', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(menu_win);

	/* Selection */
	for(;;) {
		input = getch();
		switch(input) {
		case KEY_DOWN:
			//menu_driver(menu, REQ_DOWN_ITEM);
			wrefresh(menu_win);
			break;
		case KEY_UP:
			//menu_driver(menu, REQ_UP_ITEM);
			wrefresh(menu_win);
			break;
		case KEY_ENTER:
			//choice = item_index(current_item(menu));
			goto exit_main;
		}
	}
exit_main:
#if 0
	unpost_menu(menu);
	for(i = 0; i < (menu_items_count + 1); ++i) {
		free_item(items[i]);
	}
	free(items);
	free_menu(menu);
#endif
	delwin(menu_win);

	return choice;
}

void
gui_game_scr(struct game *game)
{
	WINDOW *game_win;

	game_win = newwin(0, 0, 0, 0);

	mvwprintw(game_win, 10, 10, "in game window");
	wrefresh(game_win);
	getch();

	wclear(game_win);
	wrefresh(game_win);
	delwin(game_win);
}

void
gui_options_scr(struct game *game)
{
	WINDOW *opt_win;

	opt_win = newwin(0, 0, 0, 0);

	mvwprintw(opt_win, 10, 10, "in options window");
	wrefresh(opt_win);
	getch();

	wclear(opt_win);
	wrefresh(opt_win);
	delwin(opt_win);
}

void
gui_destroy(void)
{
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
	for (i = 0; i < game->title.len; ++i) {
		if (game->title.title[i] == '\n') {
			++row;
			col = col_start;
		}
		mvwprintw(win, row, col, "%lc", game->title.title[i]);
		col++;
	}
}
