#include "core/game.h"
#include "gui/gui.h"

struct game game;
struct game *game_p = &game;

int
main(int argc, char *argv[])
{
	struct gui gui;

	game_init(&game);
	gui_init(&gui);
	gui_run(&gui);
	gui_destroy(&gui);
	game_destroy(&game);

	return 0;
}
