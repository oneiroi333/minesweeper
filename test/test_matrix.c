#include <stdio.h>
#include "../src/core/matrix.h"

void
matrix_dump(struct matrix *m)
{
	int i, j;

	for (i = 0; i < m->rows; ++i) {
		for (j = 0; j < m->columns; ++j) {
			printf("%i ", matrix_get(m, i, j));
		}
		printf("\n");
	}
}

int
main(void)
{
	struct matrix *m1, *m2;

	printf("Create matrix m1 (5, 5, 1)\n");
	m1 = matrix_init(5, 5, 1);
	if (m1) {
		matrix_dump(m1);
		printf("Set field (2, 2) -> 3\n");
		matrix_set(m1, 2, 2, 3);
		matrix_dump(m1);
	} else {
		printf("Error: Couldnt create matrix m1\n");
	}

	printf("Create matrix m2 (15, 15, 2)\n");
	m2 = matrix_init(15, 15, 2);
	if (m2) {
		matrix_dump(m2);
		printf("Set field (10, 10) -> 9\n");
		matrix_set(m2, 10, 10, 9);
		matrix_dump(m2);
	} else {
		printf("Error: Couldnt create matrix m2\n");
	}

	matrix_free(m1);
	matrix_free(m2);

	return 0;
}


