#ifndef GAME_H
#define GAME_H

#include "matrix.h"

/* Game states */
#define GAME_STOPPED 0
#define GAME_RUNNING 1
#define GAME_PAUSED  2
#define GAME_OVER    3

/* Game outcome */
#define OUTCOME_VICTORY 0
#define OUTCOME_DEFEAT  1

/* Surface values */
#define FIELD_EMPTY      0
#define FIELD_FLAG_OFF (-1)
#define FIELD_FLAG_ON  (-2)
#define FIELD_MINE       9

/* Difficulty options */
#define DIFF_BEGINNER 0
#define DIFF_ADVANCED 1
#define DIFF_EXPERT   2
#define DIFF_CUSTOM   3

/* Grid options */
#define GRID_OFF 0
#define GRID_ON  1

struct controls {
	int up;
	int down;
	int left;
	int right;
	int reveal;
	int toggle_flag;
};

struct difficulty {
	int difficulty;
	int rows;
	int columns;
	int mines;
};

struct config {
	struct difficulty diff;
	struct controls ctrls;
};

struct playground {
	struct matrix *surface;
	struct matrix *minefield;
};

struct game {
	int state;
	int outcome;
	int fields_to_reveal;
	int fields_revealed;
	struct config cfg;
	struct playground playground;
};

void game_init(struct game *game);
void game_reinit(struct game *game);
void game_destroy(struct game *game);

struct difficulty *game_difficulty_get(struct game *game);
void game_difficulty_set(struct game *game, struct difficulty *diff);
struct controls *game_controls_get(struct game *game);
void game_controls_set(struct game *game, struct controls *controls);

void game_playground_reveal(struct game *game, int row, int column);
void game_playground_toggle_flag(struct game *game, int row, int column);
int game_playground_get(struct game *game, int row, int column);

int game_state_get(struct game *game);
int game_outcome_get(struct game *game);

#endif /* GAME_H */
