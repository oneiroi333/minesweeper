#ifndef GAME_H
#define GAME_H

#include "title.h"
#include "configuration.h"
#include "controls.h"
#include "matrix.h"

struct game {
	int state;
	int outcome;
	int fields_to_reveal;
	int fields_revealed;
	struct title title;
	struct configuration cfg;
	struct controls controls;
	struct matrix *surface;
	struct matrix *minefield;
};

void game_init(struct game *game);
void game_destroy(struct game *game);
void game_reveal(struct game *game, int row, int column);
void game_toggle_flag(struct game *game, int row, int column);

#endif /* GAME_H */
