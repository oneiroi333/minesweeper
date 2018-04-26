#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "utils.h"

int *
get_unique_rnd_array(int min, int max, int size)
{
	int *shuffle, *result, shuffle_size, tmp, rnd_idx;

	if (min > max) {
		tmp = min;
		min = max;
		max = tmp;
	}

	shuffle_size = (max - min + 1); // +1 to include max
	shuffle = (int *) malloc(shuffle_size * sizeof(*shuffle));
	result = (int *) malloc(size * sizeof(*result));

	if (!(shuffle || result)) {
		return NULL;
	}
	// Init shuffle with ascending numbers from min to max
	for (int i = 0; i < shuffle_size; i++) {
		shuffle[i] = min + i;
	}
	// Shuffle shuffle
	srand(time(NULL));
	for (int i = 0; i < shuffle_size; i++) {
		rnd_idx = min + rand() % (max + 1 - min);
		tmp = shuffle[i];
		shuffle[i] = shuffle[rnd_idx];
		shuffle[rnd_idx] = tmp;
	}
	// Fill result with the first numbers of shuffle
	for (int i = 0; i < size; i++) {
		result[i] = shuffle[i];
	}
	free(shuffle);

	return result;
}

char *
read_file(char *filename)
{
	FILE *fp;
	char *buf;
	int size, c;

	fp = fopen(filename, "r");
	if (fp == NULL ) {
		return NULL;
	}

	/* Get file size */
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = (char *) malloc(size + 1);
	if (buf != NULL) {
		fread(buf, size, 1, fp);
	}
	buf[size] = '\0';

	return buf;
}
