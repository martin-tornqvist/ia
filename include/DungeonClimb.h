#ifndef DUNGEON_CLIMB_H
#define DUNGEON_CLIMB_H

#include <iostream>

using namespace std;

class SaveHandler;

class DungeonClimb {
public:
  DungeonClimb() {}

  void tryUseDownStairs();

  void travelDown(const int levels = 1);

private:
  void makeLevel();
};

#endif
