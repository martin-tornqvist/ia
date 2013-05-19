#ifndef MARKER_H
#define MARKER_H

#include <iostream>
#include <vector>

#include "ConstTypes.h"

using namespace std;

class Engine;
class Actor;
class Item;

struct MarkerReturnData {
  MarkerReturnData() : didThrowMissile(false) {}
  bool didThrowMissile;
};

class Marker {
public:
  Marker(Engine* engine) : eng(engine) {
  }

  MarkerReturnData run(const MarkerTask_t markerTask, Item* itemThrown);

  const coord& getPos() {
    return pos_;
  }

  void draw(const MarkerTask_t markerTask) const;

private:
  coord lastKnownPlayerPos_;

  coord getClosestPos(const coord& c, const vector<coord>& positions) const;
  void setCoordToClosestEnemyIfVisible();
  bool setCoordToTargetIfVisible();
  void readKeys(const MarkerTask_t markerTask, MarkerReturnData& data,
                Item* itemThrown);
  void move(const int DX, const int DY, const MarkerTask_t markerTask,
            const Item* itemThrown);
  void cancel();
  void done();

  bool isDone_;

  coord pos_;

  Engine* eng;
};


#endif
