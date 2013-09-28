#ifndef BOT_H
#define BOT_H

#include <iostream>
#include <vector>

#include "CommonTypes.h"

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
	Pos findNextStairs();
	bool walkToAdjacentCell(const Pos& cellToGoTo);

	vector<Pos> currentPath_;
	Engine* eng;
};

#endif
