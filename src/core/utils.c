#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

int *
get_unique_rnd_array(int min, int max, int size)
{
	int  *result, i;

	result = (int *) malloc(size * sizeof(*result));
	if (result == NULL) {
		return NULL;
	}

	srand(time(NULL));
	for (i = 0; i < size; i++) {
		result[i] = min + rand() % (max + 1 - min); /* random number from min-max range */
	}

	return result;
}

char *
read_file(char *filename)
{
	FILE *fp;
	char *buf;
	int size, c;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (char *) malloc(size + 1); /* +1 for 0-terminated string*/
	if (buf != NULL) {
		fread(buf, size, 1, fp);
	}
	buf[size] = '\0';

	return buf;
}
