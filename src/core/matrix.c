#include <stdlib.h>
#include "matrix.h"

static int **matrix_table_new(int rows, int columns);
static int matrix_table_init(struct matrix *m, int rows, int columns, int init_val);
static void matrix_table_free(struct matrix *m);

struct matrix *
matrix_init(int rows, int columns, int init_val)
{
	struct matrix *m;

	m = (struct matrix *) malloc(sizeof(*m));
	if (m) {
		m->rows = rows;
		m->columns = columns;
		if (matrix_table_init(m, rows, columns, init_val) < 0) {
			free(m);
			return NULL;
		}
	}

	return m;
}

int
matrix_reinit(struct matrix *m, int rows, int columns, int init_val)
{
	int err;

	m->rows = rows;
	m->columns = columns;
	matrix_table_free(m);
	err = matrix_table_init(m, rows, columns, init_val);

	return err;
}

void
matrix_set(struct matrix *m, int row, int column, int value)
{
	m->table[row][column] = value;
}

int
matrix_get(struct matrix *m, int row, int column)
{
	return m->table[row][column];
}

void
matrix_free(struct matrix *m)
{
	matrix_table_free(m);
	free(m);
}

static int
matrix_table_init(struct matrix *m, int rows, int columns, int init_val)
{
	int i, j;

	m->table = matrix_table_new(rows, columns);
	if (m->table == NULL) {
		return -1;
	}
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			m->table[i][j] = init_val;
		}
	}

	return 0;
}

static int **
matrix_table_new(int rows, int columns)
{
	int i, err_row;
	int **table;

	table = (int **) malloc(rows * sizeof(*table));
	if (table) {
		err_row = -1;
		for (i = 0; i < rows; i++) {
			table[i] = (int *) malloc(columns * sizeof(*(table[0])));
			if (table[i] == NULL) {
				err_row = i;
				break;
			}

		}
	}
	if (err_row > -1) {
		for (i = 0; i <= err_row; i++) {
			free(table[i]);
		}
		free(table);
		return NULL;
	}

	return table;
}

static void
matrix_table_free(struct matrix *m)
{
	int i;

	for (i = 0; i < m->rows; i++) {
		free(m->table[i]);
	}
	free(m->table);
}
