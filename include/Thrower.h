#ifndef THROWER_H
#define THROWER_H

#include <iostream>
#include <vector>
#include <iostream>
#include <math.h>

#include "CommonTypes.h"

using namespace std;

class Engine;
class Item;

class Actor;

class Thrower {
public:
  Thrower(Engine& engine) : eng(engine) {}

  void throwItem(Actor& actorThrowing, const Pos& targetCell, Item& itemThrown);

  void playerThrowLitExplosive(const Pos& aimCell);

private:
  Engine& eng;
};

#endif
