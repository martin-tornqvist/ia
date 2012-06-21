#ifndef DUNGEON_CLIMB_H
#define DUNGEON_CLIMB_H

#include <iostream>

using namespace std;

class Engine;
class SaveHandler;

class DungeonClimb
{
public:
	DungeonClimb(Engine* engine) {eng = engine;}

	void attemptUseDownStairs();

	void travelDown(const int levels = 1);

private:
	Engine* eng;

	void makeLevel();
};

#endif
