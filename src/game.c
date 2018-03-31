#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "game.h"
#include "gui.h"
#include "utf8_lib.h"
#include "utils.h"

/* Textual graphic files */
#define MAIN_TITLE "media/graphic/title.ascii"

/* Game states */
#define NOT_RUNNING 0
#define RUNNING 1
#define PAUSED 2
#define GAME_OVER 3

/* Game outcome */
#define VICTORY 0
#define DEFEAT 1
#define ABORTED 2

/* Difficulty */
#define BEGINNER 0
#define ADVANCED 1
#define EXPERT 2
#define CUSTOM 3

/* Game surface field states */
#define FLAG_OFF 0
#define FLAG_ON (-1)

#define MINE 9
#define FLAG L'\u2690'

/* Game default settings */
const int default_difficulty[3][3] = {
	/* Rows, Columns, Mines */
	{10, 20, 10*20*0.4},	/* BEGINNER */
	{20, 30, 20*30*0.5},	/* ADVANCED */
	{30, 40, 30*40*0.6}	/* EXPERT */
};

const struct controls default_controls = {
	KEY_UP,		/* MOVE UP */
	KEY_DOWN,	/* MOVE DOWN */
	KEY_LEFT,	/* MOVE LEFT */
	KEY_RIGHT,	/* MOVE RIGHT */
	72,		/* REVEAL: r */
	74		/* TOGGLE FLAG: t */
};

static void game_set_difficulty(struct game *game, int difficulty, int rows, int columns, int mines);
static void game_set_controls(struct game *game, const struct controls *controls);
static void game_title_init(struct game *game);
static void game_surface_init(struct game *game);
static void game_minefield_init(struct game *game);

void
game_init(struct game *game)
{
	setlocale(LC_ALL, "");

	game->state = NOT_RUNNING;
	game->outcome = DEFEAT;
	game_set_difficulty(game, BEGINNER, 0, 0, 0);
	game->fields_to_reveal = (game->cfg.rows * game->cfg.columns) - game->cfg.mines;
	game->fields_revealed = 0;
	game_set_controls(game, &default_controls);
	game_surface_init(game);
	game_minefield_init(game);
	game_title_init(game); // should the title be part of the GUI?

	gui_init();
}

void
game_run(struct game *game)
{
	int choice;

	do {
		choice = gui_main_scr(game);
		switch(choice) {
		case PLAY:
			gui_game_scr(game);
			break;
		case OPTIONS:
			gui_options_scr(game);
			break;
		default: /* QUIT */
			;
		}
	} while (choice != QUIT);
}

void
game_reveal(struct game *game, int row, int column)
{
	int field_val;

	/* If the field has the flag set or is already revealed -> return */
	if (matrix_get_value(game->surface, row, column) != FLAG_OFF) {
		return;
	}
	field_val = matrix_get_value(game->minefield, row, column);
	if (field_val == MINE) {
		game->outcome = DEFEAT;
		game->state = GAME_OVER;
		return;
	}
	matrix_set_value(game->surface, row, column, minefield_value);
	game->fields_revealed++;
}

void
game_toggle_flag(struct game *game, int row, int column)
{
	int field_val;

	field_val = matrix_get_value(game->surface, row, column);
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
game_destroy(struct game *game)
{
	gui_destroy();

	matrix_free(game->surface);
	matrix_free(game->minefield);
	free(game->title.title);
}

static void
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

static void
game_set_controls(struct game *game, const struct controls *controls)
{
	game->controls.up = controls->up;
	game->controls.down = controls->down;
	game->controls.left = controls->left;
	game->controls.right = controls->right;
	game->controls.reveal = controls->reveal;
	game->controls.toggle_flag = controls->toggle_flag;
}

static void
game_title_init(struct game *game)
{
	char *title;
	int len;
	size_t ucs4_len;

	title = read_file(MAIN_TITLE);
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
    game->surface = matrix_init(game->difficulty.rows, game->difficulty.columns, FLAG_OFF);
}

static void
game_minefield_init(struct game *game)
{
	int *rnd_pos, row, column;

	game->minefield = matrix_init(game->difficulty.rows, game->difficulty.columns, 0);

	rnd_pos = get_unique_rnd_array(0, (rows * columns - 1), mines);
	for (int i = 0; i < mines; i++) {
		row = rnd_pos[i] / columns;
		column = rnd_pos[i] % columns;
		matrix_set_value(mfield, row, column, MINE);
	}
	free(rnd_pos);

	// Set numbers
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (matrix_get_value(mfield, i, j) != MINE) {
				// Top left corner
				if (i == 0 && j == 0) {
					if (matrix_get_value(mfield, 0, 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, 1, 0) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, 1, 1) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Top right corner
				if (i == 0 && j == (columns - 1)) {
					if (matrix_get_value(mfield, 0, columns - 2) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, 1, columns - 2) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, 1, columns - 1) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Bottom left corner
				if (i == (rows - 1) && j == 0) {
					if (matrix_get_value(mfield, rows - 2, 0) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, rows - 2, 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, rows - 1, 1) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Bottom right corner
				if (i == (rows - 1) && j == (columns - 1)) {
					if (matrix_get_value(mfield, rows - 2, columns - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, rows - 2, columns - 2) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, rows - 1, columns - 2) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Top row
				if (i == 0 && j > 0 && j < (columns - 1)) {
					if (matrix_get_value(mfield, i, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j + 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j + 1) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Right row
				if (j == (columns - 1) && i > 0 && i < (rows - 1)) {
					if (matrix_get_value(mfield, i - 1, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i - 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j) == MINE)
						matrix_incr(mfield, i, j);
				}
					// Bottom row
				if (i == (rows - 1) && j > 0 && j < (columns - 1)) {
					if (matrix_get_value(mfield, i - 1, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i - 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i - 1, j + 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j + 1) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Left row
				if (j == 0 && i > 0 && i < (rows - 1)) {
					if (matrix_get_value(mfield, i - 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i - 1, j + 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j + 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j + 1) == MINE)
						matrix_incr(mfield, i, j);
				}
				// Inner field
				if (i > 0 && i < (rows - 1) && j > 0 && j < (columns - 1)) {
					if (matrix_get_value(mfield, i - 1, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i - 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i - 1, j + 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i, j + 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j - 1) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j) == MINE)
						matrix_incr(mfield, i, j);
					if (matrix_get_value(mfield, i + 1, j + 1) == MINE)
						matrix_incr(mfield, i, j);
				}
			}
		}
	}
}
