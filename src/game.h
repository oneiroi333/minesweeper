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
void game_run(struct game *game);
void game_destroy(struct game *game);

#endif /* GAME_H */
