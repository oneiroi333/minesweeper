#ifndef CONFIG_H
#define CONFIG_H

#include "difficulty.h"
#include "controls.h"

#define PATH_CONFIG_FILE "./config"

struct config {
	struct difficulty difficulty;
	struct controls controls;
};

#endif /* CONFIG_H */
