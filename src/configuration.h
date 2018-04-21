#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/* Grid options */
#define GRID_OFF 0
#define GRID_ON 1

struct configuration {
	int difficulty;
	int rows;
	int columns;
	int mines;
	int grid;
	int grid_color;
};

#endif /* CONFIGURATION_H */
