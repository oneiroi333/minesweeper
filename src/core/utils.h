#ifndef UTILS_H
#define UTILS_H

/*
 * Create integer array filled with unique random numbers
 * min, max values are included
 */
int *get_unique_rnd_array(int min, int max, int size);

/*
 * Reads the content of a file into a dynamic allocated buffer.
 * Returns the starting address of the buffer.
 *
 * If you dont need the buffer anymore you should free it with the standard 'free' function.
 */
char *read_file(char *filename);

#endif //UTILS_H
