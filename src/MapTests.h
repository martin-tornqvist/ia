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

  void makeVisionBlockerArray(
    const coord& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS],
    const int MAX_VISION_RANMGE = FOV_MAX_RADI_INT);

  void makeMoveBlockerArray(
    const Actor* const actorMoving, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeMoveBlockerArrayForMoveType(
    const MoveType_t moveType, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeShootBlockerFeaturesArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeItemBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeMoveBlockerArrayFeaturesOnly(
    const Actor* const actorMoving, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeMoveBlockerArrayForMoveTypeFeaturesOnly(
    const MoveType_t moveType, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void addItemsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void addLivingActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void addAllActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void addAdjacentLivingActorsToBlockerArray(
    const coord origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void makeWalkBlockingArrayFeaturesOnly(
    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);

  void floodFill(
    const coord& origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
    int values[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit,
    const coord& target);

  void makeBoolVectorFromMapArray(
    bool a[MAP_X_CELLS][MAP_Y_CELLS], vector<coord>& vectorToFill);

  inline bool isCellInsideMap(const coord& pos) const {
    if(pos.x < 0) {
      return false;
    }
    if(pos.y < 0) {
      return false;
    }
    if(pos.x >= MAP_X_CELLS) {
      return false;
    }
    if(pos.y >= MAP_Y_CELLS) {
      return false;
    }
    return true;
  }

  bool isAreaInsideMap(const Rect& area) {
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

  inline bool isAreaInsideOther(const Rect& inner, const Rect& outer,
                                const bool COUNT_EQUAL_AS_INSIDE) const {
    if(COUNT_EQUAL_AS_INSIDE) {
      return
        inner.x0y0.x >= outer.x0y0.x &&
        inner.x1y1.x <= outer.x1y1.x &&
        inner.x0y0.y >= outer.x0y0.y &&
        inner.x1y1.y <= outer.x1y1.y;
    } else {
      return
        inner.x0y0.x > outer.x0y0.x &&
        inner.x1y1.x < outer.x1y1.x &&
        inner.x0y0.y > outer.x0y0.y &&
        inner.x1y1.y < outer.x1y1.y;
    }
  }

  inline bool isCellInside(const coord& pos, const Rect& area) const {
    return
      pos.x >= area.x0y0.x &&
      pos.x <= area.x1y1.x &&
      pos.y >= area.x0y0.y &&
      pos.y <= area.x1y1.y;
  }

  bool isCellNextToPlayer(const coord& pos,
                          const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;

  bool isCellsNeighbours(const coord& pos1, const coord& pos2,
                         const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;

  coord getClosestPos(const coord c, const vector<coord>& positions) const;
  Actor* getClosestActor(const coord c, const vector<Actor*>& actors) const;

  vector<coord> getLine(const coord& origin, const coord& target,
                        bool stopAtTarget, int chebTravelLimit);

  Actor* getActorAtPos(const coord pos) const;

private:
  Engine* eng;
};

#endif
