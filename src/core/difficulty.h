#ifndef DIFFICULTY_H
#define DIFFICULTY_H

/* Difficulty levels */
#define LVL_BEGINNER 0
#define LVL_ADVANCED 1
#define LVL_EXPERT   2
#define LVL_CUSTOM   3

struct difficulty {
	int lvl;
	int lvl_rows[4];
	int lvl_columns[4];
	int lvl_mines[4];
};

extern const int difficulties[3][3];

#endif /* DIFFICULTY_H */
