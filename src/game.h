#ifndef GAME_H
#define GAME_H

#include "title.h"
#include "configuration.h"
#include "controls.h"
#include "matrix.h"

/* Game states */
#define NOT_RUNNING 0
#define RUNNING 1
#define PAUSED 2
#define ABORTED 3
#define GAME_OVER 4

/* Game outcome */
#define VICTORY 0
#define DEFEAT 1

/* Difficulty */
#define BEGINNER 0
#define ADVANCED 1
#define EXPERT 2
#define CUSTOM 3

/* surface field values */
#define FLAG_OFF 0
#define FLAG_ON (-1)

/* minefield field values */
#define MINE 9

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
void game_set_difficulty(struct game *game, int difficulty, int rows, int columns, int mines);
void game_set_controls(struct game *game, const struct controls *controls);
int game_get_value(struct game *game, int row, int column);

#endif /* GAME_H */
