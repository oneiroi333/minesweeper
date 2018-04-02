#include "game.h"
#include "gui.h"

int
main(int argc, char *argv[])
{
	struct game game;
	struct gui gui;
	int choice;

	game_init(&game);
	gui_init();

	do {
		choice = gui_menu_show(&game, &gui);
		switch(choice) {
		case PLAY:
			gui_game_show(&game, &gui);
			break;
		case OPTIONS:
			gui_options_show(&game, &gui);
			break;
		default: /* QUIT */
			;
		}
	} while (choice != QUIT);

	gui_destroy(&gui);
	game_destroy(&game);

	return 0;
}
