#include <stdio.h>
#include "../src/core/matrix.h"

int
main(void)
{
	struct matrix *m;
	int i, j, val, err, error;

	error = 0;
	printf("Testing matrix... ");

	/* matrix_init */
	m = matrix_init(5, 5, 1);
	if (m) {
		if (m->rows != 5 || m->columns != 5) {
			error = 1;
		}
		for (i = 0; i < 5; ++i) {
			for (j = 0; j < 5; ++j) {
				if (m->table[i][j] != 1) {
					error = 1;
				}
			}
		}
	} else {
		error = 1;
	}
	if (error) {
		printf("matrix_init failed\n");
		goto exit_error;
	}

	/* matrix_reinit */
	err = matrix_reinit(m, 4, 4, 0);
	if (err == -1) {
		error = 1;
	} else {
		for (i = 0; i < 4; ++i) {
			for (j = 0; j < 4; ++j) {
				if (m->table[i][j] != 0) {
					error = 1;
				}
			}
		}
	}
	if (error) {
		printf("matrix_reinit failed\n");
		goto exit_error;
	}

	/* matrix_set */
	matrix_set(m, 2, 2, 3);
	if (m->table[2][2] != 3) {
			printf("matrix_set failed\n");
			goto exit_error;
	}

	/* matrix_get */
	val = matrix_get(m, 2, 2);
	if (val != 3) {
			printf("matrix_get failed\n");
			goto exit_error;
	}

	matrix_free(m);
	
	printf("OK\n");
exit_error:
	return 0;
}


