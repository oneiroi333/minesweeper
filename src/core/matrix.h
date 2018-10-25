#ifndef MATRIX_H
#define MATRIX_H

struct matrix {
	int rows;
	int columns;
	int **table;
};

/*
 * Create a matrix
 */
struct matrix *matrix_init(int rows, int columns, int initial_value);

/*
 * Reset a matrix
 * Return values:
 * 	0 OK
 * 	-1 Error
 */
int matrix_reinit(struct matrix *matrix, int rows, int columns, int initial_value);

/*
 * Setter
 */
void matrix_set(struct matrix *matrix, int row, int column, int value);

/*
 * Getter
 */
int matrix_get(struct matrix *matrix, int row, int column);

/*
 * Destroy a matrix
 */
void matrix_free(struct matrix *matrix);

#endif /* MATRIX_H */
