#ifndef THROWER_H
#define THROWER_H

#include <iostream>
#include <vector>
#include <iostream>
#include <math.h>

#include "CmnTypes.h"

using namespace std;

class Item;

class Actor;

class Thrower {
public:
  Thrower() {}

  void throwItem(Actor& actorThrowing, const Pos& targetCell, Item& itemThrown);

  void playerThrowLitExplosive(const Pos& aimCell);
};

#endif
