#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/core/utils.h"

int
main(void)
{
	int i, error;
	int *uniq_arr;
	char *filecontent;

	error = 0;
	printf("Testing utils... ");

	/* get_unique_rnd_array */
	uniq_arr = get_unique_rnd_array(1, 5, 10);
	if (uniq_arr) {
		for (i = 0; i < 10; ++i) {
			if (uniq_arr[i] < 1 || uniq_arr[i] > 5) {
				error = 1;
				break;
			}
		}
	} else {
		error = 1;
	}
	if (error) {
		printf("get_unique_rnd_array failed\n");
		goto exit_error;
	}
	free(uniq_arr);

	/* read_file */
	filecontent = read_file("test/testfile");
	if (filecontent) {
		if (strcmp(filecontent, "This is a testfile\nThis is a testfile\n") != 0) {
			error = 1;
		}
	} else {
		error = 1;
	}
	if (error) {
		printf("read_file failed\n");
		goto exit_error;
	}
	free(filecontent);

	printf("OK\n");
exit_error:
	return 0;
}


