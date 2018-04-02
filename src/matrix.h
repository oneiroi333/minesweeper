#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

struct matrix {
	int rows;
	int columns;
	int **table;
};

/*
 * Create a new matrix (two-dimensional array)
 */
struct matrix *matrix_init(int rows, int columns, int initial_value);

/*
 * Reset a given matrix
 */
void matrix_reinit(struct matrix *matrix, int rows, int columns, int initial_value);

/*
 * Set a value at a specific position in a matrix
 */
void matrix_set_value(struct matrix *matrix, int row, int column, int value);

/*
 * Get a value from a specific position in a matrix
 */
int matrix_get_value(struct matrix *matrix, int row, int column);

/*
 * Increment the value at a specific position in a matrix by 1
 */
void matrix_incr(struct matrix *matrix, int row, int column);

/*
 * Prints the matrix
 */
void matrix_dump(struct matrix *matrix, int rows, int columns, FILE *out);

/*
 * Destroy a matrix
 */
void matrix_free(struct matrix *matrix);

#endif /* MATRIX_H */
