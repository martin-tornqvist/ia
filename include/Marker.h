#ifndef MARKER_H
#define MARKER_H

#include <iostream>
#include <vector>

#include "CmnTypes.h"

class Actor;
class Item;

struct MarkerReturnData {
  MarkerReturnData() : didThrowMissile(false) {}
  bool didThrowMissile;
};

class Marker {
public:
  Marker() : eng() {}

  MarkerReturnData run(const MarkerTask markerTask, Item* itemThrown);

  const Pos& getPos() {
    return pos_;
  }

  void draw(const MarkerTask markerTask) const;

private:
  Pos lastKnownPlayerPos_;

  Pos getClosestPos(const Pos& c, const std::vector<Pos>& positions) const;
  void setPosToClosestEnemyIfVisible();
  bool setPosToTargetIfVisible();
  void readKeys(const MarkerTask markerTask, MarkerReturnData& data,
                Item* itemThrown);
  void move(const int DX, const int DY, const MarkerTask markerTask,
            const Item* itemThrown);
  void cancel();
  void done();

  bool isDone_;

  Pos pos_;


};


#endif
