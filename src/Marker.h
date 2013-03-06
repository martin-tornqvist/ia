#ifndef MARKER_H
#define MARKER_H

#include <iostream>
#include <vector>

#include "SFML/Graphics/Texture.hpp"

#include "ConstTypes.h"

using namespace std;

enum MarkerTask_t {
  markerTask_aim,
  markerTask_look,
  markerTask_throw,
  markerTask_throwLitExplosive
//  markerTask_spellAzathothsBlast,
//  markerTask_spellVoidChain
};

class Engine;
class Actor;

class Marker {
public:
  Marker(Engine* engine) : eng(engine) {
  }

  void place(const MarkerTask_t markerTask);

  const coord& getPos() {
    return pos_;
  }

private:
  coord lastKnownPlayerPos_;

  coord getClosestPos(const coord& c, const vector<coord>& positions) const;
  void setCoordToClosestEnemyIfVisible();
  bool setCoordToTargetIfVisible();

  void draw(const MarkerTask_t markerTask) const;
  void readKeys(const MarkerTask_t markerTask);
  void move(const int DX, const int DY, const MarkerTask_t markerTask);
  void cancel();
  void done();

  bool isDone_;

  coord pos_;

  Engine* eng;
};


#endif
