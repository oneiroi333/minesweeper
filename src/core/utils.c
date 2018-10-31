#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "utils.h"

#define CFG_MAX_LINE_LEN 100
#define CFG_MAX_TOKEN_LEN 10

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
	for (int i = 0; i < shuffle_size; i++) {
		shuffle[i] = min + i;
	}
	srand(time(NULL));
	for (int i = 0; i < shuffle_size; i++) {
		rnd_idx = min + rand() % (max + 1 - min);
		tmp = shuffle[i];
		shuffle[i] = shuffle[rnd_idx];
		shuffle[rnd_idx] = tmp;
	}
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

int
parse_config(struct config *cfg, char *path)
{
#if 0
	FILE *fp;
	char buf_line[CFG_MAX_LINE_LEN];
	char buf_token[CFG_MAX_TOKEN_LEN];
	char *src;

	fp = fopen(path, "r");
	if (fp == NULL ) {
		return -1;
	}
	while (get_line(buf_line, CFG_MAX_LINE_LEN, path)) {
		src = buf_line;
		src = get_token(buf_token, CFG_MAX_TOKEN_LEN, src);
		switch (buf_token) {
			case "token starts with bla do this":
				// read next value tokens
				break
			case "token that":
				// read next value tokens
				break
		}
	}
#endif

	return 0;
}

int
get_line(char *dst, int size, char *src)
{
	int nchar;

	for (nchar = 0; nchar < size - 1; ++nchar, ++dst, ++src) {
		if (*src == '\n' || *src == EOF) {
			break;
		}
		*dst = *src;
	}
	*dst = '\0';

	return nchar;
}

#if 0
char *
get_token(char *dst, int size, char *src)
{
	char *next_token_addr;
	int token_len;

	token_addr =

	return token_len;
}
#endif

