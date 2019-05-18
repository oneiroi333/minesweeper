#ifndef UTILS_H
#define UTILS_H

#include "config.h"

/*
 * Create integer array filled with unique random numbers
 * min, max values are included
 *
 * Returns a pointer to allocated memory, so you have to free it after use
 */
int *get_unique_rnd_array(int min, int max, int size);

/*
 * Reads the content of a file into a dynamic allocated buffer.
 * Returns the starting address of the buffer.
 *
 * Returns a pointer to allocated memory, so you have to free it after use
 */
char *read_file(char *filename);

#endif /* UTILS_H */
