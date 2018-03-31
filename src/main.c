#include "game.h"

int
main(int argc, char *argv[])
{
	struct game game;

	game_init(&game);
	game_run(&game);
	game_destroy(&game);

	return 0;
}
