#ifndef BOT_H
#define BOT_H

#include <iostream>
#include <vector>

#include "CmnTypes.h"

using namespace std;

class Bot {
public:
  Bot() :
    eng() {
  }

  void init();

  void act();

private:
  void findPathToStairs();
  bool walkToAdjacentCell(const Pos& cellToGoTo);

  vector<Pos> currentPath_;
};

#endif
