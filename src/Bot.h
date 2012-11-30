#ifndef BOT_H
#define BOT_H

#include <iostream>
#include <vector>

#include "ConstTypes.h"

using namespace std;

class Engine;

class Bot {
public:
	Bot(Engine* engine) :
		eng(engine) {
	}

	void init();

	void act();

private:
	void findPathToNextStairs();
	coord findNextStairs();
	bool walkToAdjacentCell(const coord& cellToGoTo);

	void runFunctionTests();

	vector<coord> currentPath_;
	Engine* eng;
};

#endif
