#ifndef MAP_TESTS_H
#define MAP_TESTS_H

#include <vector>

#include "ConstTypes.h"
#include "Config.h"

class Engine;
class TimedEntity;
class Actor;
class Feature;

//Function object for sorting containers by distance to origin
struct IsCloserToOrigin {
public:
  IsCloserToOrigin(const coord& c, const Engine* engine) :
    c_(c), eng(engine) {
  }
  bool operator()(const coord& c1, const coord& c2);
  coord c_;
  const Engine* eng;
};

class MapTests {
public:
  MapTests(Engine* engine) {
    eng = engine;
  }

  void makeVisionBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void makeMoveBlockerArray(const Actor* const actorMoving, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void makeMoveBlockerArrayForMoveType(const MoveType_t moveType, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void makeShootBlockerFeaturesArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void makeItemBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void makeMoveBlockerArrayFeaturesOnly(const Actor* const actorMoving, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void makeMoveBlockerArrayForMoveTypeFeaturesOnly(const MoveType_t moveType, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void addItemsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void addLivingActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void addAllActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  void addAdjacentLivingActorsToBlockerArray(const coord origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
  //Mostly for map generation
  void makeWalkBlockingArrayFeaturesOnly(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeFloodFill(const coord origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS], int valueArray[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit,
                     const coord target);

  void makeMapVectorFromArray(bool a[MAP_X_CELLS][MAP_Y_CELLS], vector<coord>& vectorToFill);

  bool isCellInsideMainScreen(const int x, const int y) const {
    if(x < 0) {
      return false;
    }
    if(y < 0) {
      return false;
    }
    if(x >= MAP_X_CELLS) {
      return false;
    }
    if(y >= MAP_Y_CELLS) {
      return false;
    }
    return true;
  }

  bool isAreaInsideMainScreen(const Rect& area) {
    if(area.x0y0.x < 0) {
      return false;
    }
    if(area.x0y0.y < 0) {
      return false;
    }
    if(area.x1y1.x >= MAP_X_CELLS) {
      return false;
    }
    if(area.x1y1.y >= MAP_Y_CELLS) {
      return false;
    }
    return true;
  }

  bool isCellInside(const coord& pos, const coord& x0y0, const coord& x1y1) {
    return isCellInside(pos, Rect(x0y0, x1y1));
  }

  bool isCellInside(const coord& pos, const Rect& area) {
    return pos.x >= area.x0y0.x && pos.x <= area.x1y1.x && pos.y >= area.x0y0.y && pos.y <= area.x1y1.y;
  }

  bool isCellInsideMainScreen(const coord& c) const {
    return isCellInsideMainScreen(c.x, c.y);
  }

  bool isCellNextToPlayer(const int x, const int y, const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;

  bool isCellsNeighbours(const int x1, const int y1, const int x2, const int y2, const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;
  bool isCellsNeighbours(const coord c1, const coord c2, const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;

  coord getClosestPos(const coord c, const vector<coord>& positions) const;
  Actor* getClosestActor(const coord c, const vector<Actor*>& actors) const;

  //getLine stops at first travel limiter (stop at target, max travel distance.
  vector<coord> getLine(int originX, int originY, int targetX, int targetY, bool stopAtTarget, int chebTravelLimit);

  Actor* getActorAtPos(const coord pos) const;

private:
  Engine* eng;
};

#endif
