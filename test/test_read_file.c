#include <stdio.h>
#include <stdlib.h>
#include "../src/utils.h"

int
main(void)
{
	char *file1, *file2;

	file1 = read_file("../ascii/title.ascii");
	file2 = read_file("../ascii/title2.ascii");

	printf("%s", file1);
	printf("\n================================================\n");
	printf("%s", file2);

	free(file1);
	free(file2);

	return 0;
}
