#include "core/game.h"
#include "gui/gui.h"

int
main(int argc, char *argv[])
{
	struct game game;
	struct gui gui;
	int choice, first;

	game_init(&game);
	gui_init(&gui);
	gui_start(&gui);
	gui_destroy(&gui);
	game_destroy(&game);

	return 0;
}

#if 0
	first = 1;
	do {
		choice = gui_menu_show(&game, &gui);
		switch(choice) {
			case PLAY:
				game.state = RUNNING;
				gui_game_show(&game, &gui);
				if (!first) {
					game_reinit(&game);
				}
				first = 0;
				break;
			case OPTIONS:
				gui_options_show(&game, &gui);
				break;
			default: /* QUIT */
				;
		}
	} while (choice != QUIT);
#endif

