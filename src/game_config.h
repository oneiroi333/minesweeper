/* Game default settings */
const int default_difficulty[3][3] = {
	/* Rows, Columns, Mines */
	{10, 20, 10*20*0.2},	/* BEGINNER */
	{20, 30, 20*30*0.3},	/* ADVANCED */
	{30, 40, 30*40*0.35}	/* EXPERT */
};

const struct controls default_controls = {
	KEY_UP,		/* MOVE UP */
	KEY_DOWN,	/* MOVE DOWN */
	KEY_LEFT,	/* MOVE LEFT */
	KEY_RIGHT,	/* MOVE RIGHT */
	114,		/* REVEAL */ /* r */
	116		/* TOGGLE FLAG */ /* t */
#if 0
	/* vim like */
	107, /* k */
	106, /* j */
	104, /* h */
	108, /* l */
	100, /* d */
	102 /* f */
#endif
};
