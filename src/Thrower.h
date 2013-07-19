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

//struct ThrownObject {
////  double xIncr, yIncr, xPos_prec, yPos_prec, hypot;
//  Pos pos;
//  Pos aimPos;
//  int cellPotential;
//  vector<Pos> flightPath;
//  //  Pos blockedByCell;
//};

class Thrower {
public:
  Thrower(Engine* engine) :
    eng(engine) {
  }

  void throwItem(Actor& actorThrowing, const Pos& targetCell,
                 Item& itemThrown);

  void playerThrowLitExplosive(const Pos& aimCell);

private:
  Engine* eng;
};

#endif
