#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "game.h"
#include "gui.h"
#include "utf8_lib.h"
#include "utils.h"

/* Textual graphic files */
#define PATH_TITLE "media/graphic/title"
#define PATH_SKULL "media/graphic/skull"

/* Game default settings */
const int default_difficulty[3][3] = {
	/* Rows, Columns, Mines */
	{10, 20, 10*20*0.2},	/* BEGINNER */
	{20, 30, 20*30*0.3},	/* ADVANCED */
	{30, 40, 30*40*0.35}	/* EXPERT */
};

const struct controls default_controls = {
#if 0
	KEY_UP,		/* MOVE UP */
	KEY_DOWN,	/* MOVE DOWN */
	KEY_LEFT,	/* MOVE LEFT */
	KEY_RIGHT,	/* MOVE RIGHT */
	114,		/* REVEAL */ /* r */
	116		/* TOGGLE FLAG */ /* t */
#endif
	/* vim like */
	107, /* k */
	106, /* j */
	104, /* h */
	108, /* l */
	100, /* d */
	102 /* f */
};

static void game_title_init(struct game *game);
static void game_surface_init(struct game *game);
static void game_surface_reinit(struct game *game);
static void game_surface_expand_empty_field(struct game *game, int row, int col);
static void game_minefield_init(struct game *game);
static void game_minefield_reinit(struct game *game);
static void game_minefield_set_numbers(struct game *game);
//static int game_get_dead_field_count(struct game *game);

void
game_init(struct game *game)
{
	setlocale(LC_ALL, "");

	game->state = NOT_RUNNING;
	game->outcome = DEFEAT;
	game_set_difficulty(game, BEGINNER, 0, 0, 0);
	game->cfg.grid = GRID_ON;
	game_set_controls(game, &default_controls);
	game_surface_init(game);
	game_minefield_init(game);
#if 0
	game->fields_to_reveal = (game->cfg.rows * game->cfg.columns) - game->cfg.mines - game_get_dead_field_count(game);
	game->fields_revealed = 0;
#endif
	game_title_init(game);
}

void
game_reinit(struct game *game)
{
	game_surface_reinit(game);
	game_minefield_reinit(game);
#if 0
	game->fields_to_reveal = (game->cfg.rows * game->cfg.columns) - game->cfg.mines - game_get_dead_field_count(game);
	game->fields_revealed = 0;
#endif
}

void
game_destroy(struct game *game)
{
	matrix_free(game->surface);
	matrix_free(game->minefield);
	free(game->title.title);
}

void
game_reveal(struct game *game, int row, int column)
{
	int field_val;

	/* The field is already revealed */
	if (matrix_get_value(game->surface, row, column) >= 0) {
		return;
	}

	field_val = matrix_get_value(game->minefield, row, column);
	if (field_val == MINE) {
		game->outcome = DEFEAT;
		game->state = GAME_OVER;
		return;
	}
	if (field_val == FIELD_EMPTY) {
		// expand empty fields
		matrix_set_value(game->surface, row, column, field_val);
	} else {
		matrix_set_value(game->surface, row, column, field_val);
		/*
		game->fields_revealed++;
		if (game->fields_revealed == game->fields_to_reveal) {
			game->state = GAME_OVER;
			game->outcome = VICTORY;
		}
		*/
	}
}

void
game_toggle_flag(struct game *game, int row, int column)
{
	int field_val;

	field_val = matrix_get_value(game->surface, row, column);
	/* Field already revealed */
	if (field_val >= 0) {
		return;
	}
	switch (field_val) {
	case FLAG_OFF:
		matrix_set_value(game->surface, row, column, FLAG_ON);
		break;
	case FLAG_ON:
		matrix_set_value(game->surface, row, column, FLAG_OFF);
		break;
	default: /* REVEALED */
		;
	}
}

void
game_set_difficulty(struct game *game, int difficulty, int rows, int columns, int mines)
{
	game->cfg.difficulty = difficulty;
	if (difficulty != CUSTOM) {
		game->cfg.rows = default_difficulty[difficulty][0];
		game->cfg.columns = default_difficulty[difficulty][1];
		game->cfg.mines = default_difficulty[difficulty][2];
	} else {
		game->cfg.rows = rows;
		game->cfg.columns = columns;
		game->cfg.mines = mines;
	}
}

void
game_set_controls(struct game *game, const struct controls *controls)
{
	game->controls.up = controls->up;
	game->controls.down = controls->down;
	game->controls.left = controls->left;
	game->controls.right = controls->right;
	game->controls.reveal = controls->reveal;
	game->controls.toggle_flag = controls->toggle_flag;
}

int
game_get_value(struct game *game, int row, int column)
{
	return matrix_get_value(game->surface, row, column);
}

static void
game_title_init(struct game *game)
{
	char *title;
	int len;
	size_t ucs4_len;

	title = read_file(PATH_TITLE);
	len = strlen(title);
	if (is_valid_utf8(title, len)) {
		game->title.title = utf8_to_ucs4(title, len, &ucs4_len);
		game->title.len = ucs4_len;
	} else {
		game->title.title = NULL;
		game->title.len = 0;
	}
}

static void
game_surface_init(struct game *game)
{
	game->surface = matrix_init(game->cfg.rows, game->cfg.columns, FLAG_OFF);
}

static void
game_surface_reinit(struct game *game)
{
	int row, col;

	for (row = 0; row < game->cfg.rows; ++row) {
		for (col = 0; col < game->cfg.columns; ++col) {
			matrix_set_value(game->surface, row, col, FLAG_OFF);
		}
	}
}

static void
game_minefield_init(struct game *game)
{
	int *rnd_pos;
	int i, j, row, column;

	game->minefield = matrix_init(game->cfg.rows, game->cfg.columns, FIELD_EMPTY);

	rnd_pos = get_unique_rnd_array(0, (game->cfg.rows * game->cfg.columns - 1), game->cfg.mines);
	for (i = 0; i < game->cfg.mines; ++i) {
		row = rnd_pos[i] / game->cfg.columns;
		column = rnd_pos[i] % game->cfg.columns;
		matrix_set_value(game->minefield, row, column, MINE);
	}
	free(rnd_pos);

	game_minefield_set_numbers(game);
}

static void
game_minefield_reinit(struct game *game)
{
	int i, row, column, *rnd_pos;

	rnd_pos = get_unique_rnd_array(0, (game->cfg.rows * game->cfg.columns - 1), game->cfg.mines);
	for (row = 0; row < game->cfg.rows; ++row) {
		for (column = 0; column < game->cfg.columns; ++column) {
			matrix_set_value(game->minefield, row, column, FIELD_EMPTY);
		}
	}
	for (i = 0; i < game->cfg.mines; ++i) {
		row = rnd_pos[i] / game->cfg.columns;
		column = rnd_pos[i] % game->cfg.columns;
		matrix_set_value(game->minefield, row, column, MINE);
	}
	free(rnd_pos);
	game_minefield_set_numbers(game);
}

static void
game_minefield_set_numbers(struct game *game)
{
	int i, j;

	for (i = 0; i < game->cfg.rows; ++i) {
		for (j = 0; j < game->cfg.columns; ++j) {
			if (matrix_get_value(game->minefield, i, j) != MINE) {
				// Top left corner
				if (i == 0 && j == 0) {
					if (matrix_get_value(game->minefield, 0, 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, 1, 0) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, 1, 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Top right corner
				if (i == 0 && j == (game->cfg.columns - 1)) {
					if (matrix_get_value(game->minefield, 0, game->cfg.columns - 2) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, 1, game->cfg.columns - 2) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, 1, game->cfg.columns - 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Bottom left corner
				if (i == (game->cfg.rows - 1) && j == 0) {
					if (matrix_get_value(game->minefield, game->cfg.rows - 2, 0) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, game->cfg.rows - 2, 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, game->cfg.rows - 1, 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Bottom right corner
				if (i == (game->cfg.rows - 1) && j == (game->cfg.columns - 1)) {
					if (matrix_get_value(game->minefield, game->cfg.rows - 2, game->cfg.columns - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, game->cfg.rows - 2, game->cfg.columns - 2) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, game->cfg.rows - 1, game->cfg.columns - 2) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Top row
				if (i == 0 && j > 0 && j < (game->cfg.columns - 1)) {
					if (matrix_get_value(game->minefield, i, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Right row
				if (j == (game->cfg.columns - 1) && i > 0 && i < (game->cfg.rows - 1)) {
					if (matrix_get_value(game->minefield, i - 1, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i - 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Bottom row
				if (i == (game->cfg.rows - 1) && j > 0 && j < (game->cfg.columns - 1)) {
					if (matrix_get_value(game->minefield, i - 1, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i - 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i - 1, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Left row
				if (j == 0 && i > 0 && i < (game->cfg.rows - 1)) {
					if (matrix_get_value(game->minefield, i - 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i - 1, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
				// Inner field
				if (i > 0 && i < (game->cfg.rows - 1) && j > 0 && j < (game->cfg.columns - 1)) {
					if (matrix_get_value(game->minefield, i - 1, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i - 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i - 1, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j - 1) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j) == MINE)
						matrix_incr(game->minefield, i, j);
					if (matrix_get_value(game->minefield, i + 1, j + 1) == MINE)
						matrix_incr(game->minefield, i, j);
				}
			}
		}
	}
}

static void
game_surface_expand_empty_field(struct game *game, int row, int col)
{
}

#if 0
static int
game_get_dead_field_count(struct game *game)
{
	int i, j, cnt;

	cnt = 0;
	for(i = 0; i < game->cfg.rows; ++i) {
		for (j = 0; j < game->cfg.columns; ++j) {
			if (!matrix_get_value(game->minefield, i, j)) {
				++cnt;
			}
		}
	}

	return cnt;
}
#endif
