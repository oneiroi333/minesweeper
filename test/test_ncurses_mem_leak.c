#include <stdio.h>
#include <ncurses.h>

int
main(void)
{
	WINDOW *win;

	initscr();

	addch('T');
	refresh();
	getch();

	endwin();
}
