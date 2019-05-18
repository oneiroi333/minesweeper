#include <stdlib.h>
#include "game.h"
#include "utils.h"

#define PATH_CONFIG_FILE "config"

static void game_state_init(struct game *game);
static void game_state_reinit(struct game *game);
static void game_config_init(struct game *game);
static void game_playground_init(struct game *game);
static void game_playground_reinit(struct game *game);
static void game_playground_destroy(struct game *game);
static void game_surface_init(struct game *game);
static void game_surface_reinit(struct game *game);
static void game_minefield_init(struct game *game);
static void game_minefield_reinit(struct game *game);
static void game_minefield_set_numbers(struct game *game);

void
game_init(struct game *game)
{
	game_config_init(game);
	game_state_init(game);
	game_playground_init(game);
}

void
game_reinit(struct game *game)
{
	game_state_reinit(game);
	game_playground_reinit(game);
}

void
game_destroy(struct game *game)
{
	matrix_free(game->playground.surface);
	matrix_free(game->playground.minefield);
}

void
game_start(struct game *game)
{
	game->game_state.state = GAME_RUNNING;
}

void
game_quit(struct game *game)
{
	game_state_reinit(game);
}

int
game_playground_reveal(struct game *game, int row, int column)
{
	int field_val, i, j, nmines;

	if (matrix_get(game->playground.surface, row, column) >= 0) {
		return FIELD_REVEALED;
	}
	/*
	 * Update game state, check win condition
	 */
	field_val = matrix_get(game->playground.minefield, row, column);
	if (field_val > 0) {
		if (field_val != FIELD_MINE) {
			matrix_set(game->playground.surface, row, column, field_val);
			game->game_state.fields_revealed++;
			if (game->game_state.fields_revealed == game->game_state.fields_to_reveal) {
				game->game_state.state = GAME_OVER;
				game->game_state.outcome = OUTCOME_VICTORY;
			}
		} else {
			game->game_state.state = GAME_OVER;
			game->game_state.outcome = OUTCOME_DEFEAT;
		}
		return field_val;
	}
	/*
	 * as long as we get an empty field call the reveal funtion
	 *
	 * If the field is an empty field recursively reveal the empty fields connected to it
	 */
	if (field_val == FIELD_EMPTY) {
		// TODO recursive reveal:
		// for every neighbour cell
		// 	if field is a number
		// 		reveal the field
		// 	else if the field is empty
		// 		call this function for the field
		matrix_set(game->playground.surface, row, column, field_val);
		game->game_state.fields_revealed++;
		return FIELD_EMPTY; // delete after implementation
	}
	/* TODO More then one surface field has been updated */
	return FIELD_MULTIPLE;
}

void
game_playground_toggle_flag(struct game *game, int row, int column)
{
	int field_val;

	field_val = matrix_get(game->playground.surface, row, column);
	switch (field_val) {
	case FIELD_FLAG_OFF:
		matrix_set(game->playground.surface, row, column, FIELD_FLAG_ON);
		break;
	case FIELD_FLAG_ON:
		matrix_set(game->playground.surface, row, column, FIELD_FLAG_OFF);
		break;
	default: /* REVEALED */
		;
	}
}

int
game_playground_get(struct game *game, int row, int column)
{
	return matrix_get(game->playground.surface, row, column);
}

int
game_playground_get_pos_y_player(struct game *game)
{
	return game->playground.pos_y_player;
}

int
game_playground_get_pos_x_player(struct game *game)
{
	return game->playground.pos_x_player;
}

void
game_playground_set_pos_y_player(struct game *game, int pos_y)
{
	game->playground.pos_y_player = pos_y;
}

void
game_playground_set_pos_x_player(struct game *game, int pos_x)
{
	game->playground.pos_x_player = pos_x;
}

struct game_state *
game_state_get(struct game *game)
{
	return &game->game_state;
}

void
game_config_difficulty_set(struct game *game, struct difficulty *difficulty)
{
	game->config.difficulty.lvl = difficulty->lvl;
	if (difficulty->lvl == LVL_CUSTOM) {
		game->config.difficulty.lvl_rows[LVL_CUSTOM] = difficulty->lvl_rows[LVL_CUSTOM];
		game->config.difficulty.lvl_columns[LVL_CUSTOM] = difficulty->lvl_columns[LVL_CUSTOM];
		game->config.difficulty.lvl_mines[LVL_CUSTOM] = difficulty->lvl_mines[LVL_CUSTOM];
	}
}

struct difficulty *
game_config_difficulty_get(struct game *game)
{
	return &game->config.difficulty;
}

void
game_config_controls_set(struct game *game, struct controls *controls)
{
	game->config.controls.up = controls->up;
	game->config.controls.down = controls->down;
	game->config.controls.left = controls->left;
	game->config.controls.right = controls->right;
	game->config.controls.reveal = controls->reveal;
	game->config.controls.toggle_flag = controls->toggle_flag;
}

struct controls *
game_config_controls_get(struct game *game)
{
	return &game->config.controls;
}

static void
game_state_init(struct game *game)
{
	int lvl;

	lvl = game->config.difficulty.lvl;

	game->game_state.state = GAME_ABORTED;
	game->game_state.outcome = OUTCOME_DEFEAT;
	game->game_state.fields_revealed = 0;
	game->game_state.fields_to_reveal = (game->config.difficulty.lvl_rows[lvl] * game->config.difficulty.lvl_columns[lvl]) - game->config.difficulty.lvl_mines[lvl];
}

static void
game_state_reinit(struct game *game)
{
	game_state_init(game);
}

static void
game_config_init(struct game *game)
{
	// TODO Read config file
	//
	game->config.controls.up = 107;
	game->config.controls.down = 106;
	game->config.controls.left = 104;
	game->config.controls.right = 108;
	game->config.controls.reveal = 100;
	game->config.controls.toggle_flag = 102;
	//
	//
	int lvl;

	game->config.difficulty.lvl = LVL_BEGINNER;
	lvl = game->config.difficulty.lvl;
	game->config.difficulty.lvl_rows[lvl] = 8;
	game->config.difficulty.lvl_columns[lvl] = 12;
	game->config.difficulty.lvl_mines[lvl] = 25;
}

static void
game_playground_init(struct game *game)
{
	game_surface_init(game);
	game_minefield_init(game);
	game->playground.pos_y_player = 0;
	game->playground.pos_x_player = 0;
}

static void
game_playground_reinit(struct game *game)
{
	game_surface_reinit(game);
	game_minefield_reinit(game);
	game->playground.pos_y_player = 0;
	game->playground.pos_x_player = 0;
}

static void
game_surface_init(struct game *game)
{
	int rows, columns;

	rows = game->config.difficulty.lvl_rows[game->config.difficulty.lvl];
	columns = game->config.difficulty.lvl_columns[game->config.difficulty.lvl];
	game->playground.surface = matrix_init(rows, columns, FIELD_FLAG_OFF);
}

static void
game_surface_reinit(struct game *game)
{
	int row, col;

	for (row = 0; row < game->config.difficulty.lvl_rows[game->config.difficulty.lvl]; ++row) {
		for (col = 0; col < game->config.difficulty.lvl_columns[game->config.difficulty.lvl]; ++col) {
			matrix_set(game->playground.surface, row, col, FIELD_FLAG_OFF);
		}
	}
}

static void
game_minefield_init(struct game *game)
{
	int *rnd_pos;
	int i, j;
	int row, column;
	int rows, columns, lvl;

	lvl = game->config.difficulty.lvl;
	rows = game->config.difficulty.lvl_rows[lvl];
	columns = game->config.difficulty.lvl_columns[lvl];
	game->playground.minefield = matrix_init(rows, columns, FIELD_EMPTY);

	rnd_pos = get_unique_rnd_array(0, (rows * columns - 1), game->config.difficulty.lvl_mines[lvl]);
	for (i = 0; i < game->config.difficulty.lvl_mines[lvl]; ++i) {
		row = rnd_pos[i] / game->config.difficulty.lvl_columns[lvl];
		column = rnd_pos[i] % game->config.difficulty.lvl_columns[lvl];
		matrix_set(game->playground.minefield, row, column, FIELD_MINE);
	}
	free(rnd_pos);
	game_minefield_set_numbers(game);
}

static void
game_minefield_reinit(struct game *game)
{
	int i, row, column, lvl, *rnd_pos;

	lvl = game->config.difficulty.lvl;
	rnd_pos = get_unique_rnd_array(0, (game->config.difficulty.lvl_rows[lvl] * game->config.difficulty.lvl_columns[lvl] - 1), game->config.difficulty.lvl_mines[lvl]);
	for (row = 0; row < game->config.difficulty.lvl_rows[lvl]; ++row) {
		for (column = 0; column < game->config.difficulty.lvl_columns[lvl]; ++column) {
			matrix_set(game->playground.minefield, row, column, FIELD_EMPTY);
		}
	}
	for (i = 0; i < game->config.difficulty.lvl_mines[lvl]; ++i) {
		row = rnd_pos[i] / game->config.difficulty.lvl_columns[lvl];
		column = rnd_pos[i] % game->config.difficulty.lvl_columns[lvl];
		matrix_set(game->playground.minefield, row, column, FIELD_MINE);
	}
	free(rnd_pos);
	game_minefield_set_numbers(game);
}

static void
game_minefield_set_numbers(struct game *game)
{
	int i, j;
	int lvl, lvl_rows, lvl_columns;

	lvl = game->config.difficulty.lvl;
	lvl_rows = game->config.difficulty.lvl_rows[lvl];
	lvl_columns = game->config.difficulty.lvl_columns[lvl];
	for (i = 0; i < lvl_rows; ++i) {
		for (j = 0; j < lvl_columns; ++j) {
			if (matrix_get(game->playground.minefield, i, j) != FIELD_MINE) {
				// Top left corner
				if (i == 0 && j == 0) {
					if (matrix_get(game->playground.minefield, 0, 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, 1, 0) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, 1, 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Top right corner
				if (i == 0 && j == (lvl_columns - 1)) {
					if (matrix_get(game->playground.minefield, 0, game->config.difficulty.lvl_columns[lvl] - 2) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, 1, game->config.difficulty.lvl_columns[lvl] - 2) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, 1, game->config.difficulty.lvl_columns[lvl] - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Bottom left corner
				if (i == (lvl_rows - 1) && j == 0) {
					if (matrix_get(game->playground.minefield, game->config.difficulty.lvl_rows[lvl] - 2, 0) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, game->config.difficulty.lvl_rows[lvl] - 2, 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, game->config.difficulty.lvl_rows[lvl] - 1, 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Bottom right corner
				if (i == (lvl_rows - 1) && j == (lvl_columns - 1)) {
					if (matrix_get(game->playground.minefield, game->config.difficulty.lvl_rows[lvl] - 2, game->config.difficulty.lvl_columns[lvl] - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, game->config.difficulty.lvl_rows[lvl] - 2, game->config.difficulty.lvl_columns[lvl] - 2) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, game->config.difficulty.lvl_rows[lvl] - 1, game->config.difficulty.lvl_columns[lvl] - 2) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Top row
				if (i == 0 && j > 0 && j < (lvl_columns - 1)) {
					if (matrix_get(game->playground.minefield, i, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Right row
				if (j == (lvl_columns - 1) && i > 0 && i < (lvl_rows - 1)) {
					if (matrix_get(game->playground.minefield, i - 1, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i - 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Bottom row
				if (i == (lvl_rows - 1) && j > 0 && j < (lvl_columns - 1)) {
					if (matrix_get(game->playground.minefield, i - 1, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i - 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i - 1, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Left row
				if (j == 0 && i > 0 && i < (lvl_rows - 1)) {
					if (matrix_get(game->playground.minefield, i - 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i - 1, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
				// Inner field
				if (i > 0 && i < (lvl_rows - 1) && j > 0 && j < (lvl_columns - 1)) {
					if (matrix_get(game->playground.minefield, i - 1, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i - 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i - 1, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j - 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
					if (matrix_get(game->playground.minefield, i + 1, j + 1) == FIELD_MINE)
						matrix_set(game->playground.minefield, i, j, matrix_get(game->playground.minefield, i, j) + 1);
				}
			}
		}
	}
}

int
game_parse_config_file(struct config *cfg, char *path)
{

#if 0
#define CFG_MAX_LINE_LEN 100
#define CFG_MAX_TOKEN_LEN 10

	FILE *fp;
	char buf_line[CFG_MAX_LINE_LEN];
	char buf_token[CFG_MAX_TOKEN_LEN];
	char *src;

	fp = fopen(path, "r");
	if (fp == NULL ) {
		return -1;
	}
	while (get_line(buf_line, CFG_MAX_LINE_LEN, path)) {
		src = buf_line;
		src = get_token(buf_token, CFG_MAX_TOKEN_LEN, src);
		switch (buf_token) {
			case "token starts with bla do this":
				// read next value tokens
				break
			case "token that":
				// read next value tokens
				break
		}
	}
#endif

	return 0;
}
