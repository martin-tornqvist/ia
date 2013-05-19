#ifndef THROWER_H
#define THROWER_H

#include <iostream>
#include <vector>
#include <iostream>
#include <math.h>

#include "ConstTypes.h"

using namespace std;

class Engine;
class Item;

class Actor;

//struct ThrownObject {
////  double xIncr, yIncr, xPos_prec, yPos_prec, hypot;
//  coord pos;
//  coord aimPos;
//  int cellPotential;
//  vector<coord> flightPath;
//  //  coord blockedByCell;
//};

class Thrower {
public:
  Thrower(Engine* engine) :
    eng(engine) {
  }

  void throwItem(Actor& actorThrowing, const coord& targetCell,
                 Item& itemThrown);

  void playerThrowLitExplosive(const coord& aimCell);

private:
  Engine* eng;
};

#endif
