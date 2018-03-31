#include <stdio.h>
#include "../src/matrix.c"

int
main(void)
{
	struct matrix *m;

	printf("=== Starting test: matrix ===\n");

	printf("Initializing matrix...\n");
	m = matrix_init(10, 20, 1);

	printf("Dumping matrix...\n");
	matrix_dump(m, 10, 20, stdout);

	printf("Reinitializing matrix...\n");
	matrix_reinit(m, 10, 18, 28, 4),

	printf("Dumping matrix...\n");
	matrix_dump(m, 18, 28, stdout);

	printf("Setting value of matrix @{row: 1, col: 1, val: 99}...\n");
	matrix_set_value(m, 1, 1, 99);
	printf("Value set @row=1, col=1, value=99\n");
	printf("Value @row=1, col=1, value=%d\n", matrix_get_value(m, 1, 1));

	printf("Incrementing value of matrix @{row: 1, col: 1}...\n");
	matrix_incr(m, 1, 1);
	printf("Value incremented @row=1, col=1\n");
	printf("Value @row=1, col=1, value=%d\n", matrix_get_value(m, 1, 1));

	printf("Freeing matrix...\n");
	matrix_free(m, 18);

	printf("=== Finished test: matrix ===\n");

	return 0;
}
