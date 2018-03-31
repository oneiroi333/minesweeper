#ifndef TITLE_H
#define TITLE_H

#include <stdint.h>

struct title {
	uint32_t *title;
	size_t len;
	size_t width;
	size_t height;
};

#endif /* TITLE_H */
