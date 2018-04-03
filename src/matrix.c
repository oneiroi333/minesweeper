#include <stdlib.h>
#include <assert.h>
#include "matrix.h"

static struct matrix *matrix_new(void);
static void matrix_table_init(struct matrix *m, int rows, int columns, int initial_value);
static int **matrix_table_new(int rows, int columns);
static void matrix_table_free(struct matrix *m);

struct matrix *
matrix_init(int rows, int columns, int initial_value)
{
	struct matrix *m = matrix_new();
	matrix_table_init(m, rows, columns, initial_value);

	return m;
}

void
matrix_reinit(struct matrix *m, int rows, int columns, int initial_value)
{
	matrix_table_free(m);
	matrix_table_init(m, rows, columns, initial_value);
}

void
matrix_set_value(struct matrix *m, int row, int column, int value)
{
	m->table[row][column] = value;
}

int
matrix_get_value(struct matrix *m, int row, int column)
{
	return m->table[row][column];
}

void
matrix_incr(struct matrix *m, int row, int column)
{
	m->table[row][column]++;
}

void
matrix_dump(struct matrix *m, int rows, int columns, FILE *out)
{
	int i, j;

	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			fprintf(out, "%d ", m->table[i][j]);
		}
		fprintf(out, "\n");
	}
}

void
matrix_free(struct matrix *m)
{
	matrix_table_free(m);
	free(m);
}

static void
matrix_table_init(struct matrix *m, int rows, int columns, int initial_value)
{
	m->table = matrix_table_new(rows, columns);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			m->table[i][j] = initial_value;
		}
	}
}

static int **
matrix_table_new(int rows, int columns)
{
	int i;
	int **table = (int **) malloc(rows * sizeof(*table));
	assert(table);

	for (i = 0; i < rows; i++) {
		table[i] = (int *) malloc(columns * sizeof(*(table[0])));
		assert(table[i]);
	}

	return table;
}

static void
matrix_table_free(struct matrix *m)
{
	for (int i = 0; i < m->rows; i++) {
		free(m->table[i]);
	}
	free(m->table);
}

static struct matrix *
matrix_new(void)
{
	struct matrix *m = (struct matrix *) malloc(sizeof(*m));
	assert(m);

	return m;
}
