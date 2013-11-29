#ifndef MAP_TESTS_H
#define MAP_TESTS_H

#include <vector>

#include "CommonTypes.h"
#include "Config.h"

class Engine;

enum CellPredCheckEntity_t {
  cellPredCheckEntityActor,
  cellPredCheckEntityFeatureStatic,
  cellPredCheckEntityFeatureMob
};

class CellPred {
public:
  CellPred(Engine* engine) : eng(engine) {}
  virtual bool isCheckingCells()          const = 0;
  virtual bool isCheckingMobFeatures()    const = 0;
  virtual bool isCheckingActors()         const = 0;
  virtual bool check(const Cell& c)       const = 0;
  virtual bool check(const FeatureMob& f) const = 0;
  virtual bool check(const Actor& a)      const = 0;
  const Engine* const eng;
};

class CellPredBlocksVision : public CellPred {
public:
  CellPredBlocksVision(Engine* engine) : CellPred(engine) {}
  bool isCheckingCells()          const {return true;}
  bool isCheckingMobFeatures()    const {return true;}
  bool isCheckingActors()         const {return false;}
  bool check(const Cell& c)       const;
  bool check(const FeatureMob& f) const;
  bool check(const Actor& a)      const {(void)a; return false;}
};

class CellPredBlocksBodyType : public CellPred {
public:
  CellPredBlocksBodyType(BodyType_t bodyType, Engine* engine) :
    CellPred(engine), bodyType_(bodyType) {}
  bool isCheckingCells()          const {return true;}
  bool isCheckingMobFeatures()    const {return true;}
  bool isCheckingActors()         const {return true;}
  bool check(const Cell& c)       const;
  bool check(const FeatureMob& f) const;
  bool check(const Actor& a)      const;
  BodyType_t bodyType_;
};

enum MapParseWriteRule {
  mapParseWriteAlways,
  mapParseWriteOnlyFalse,
  mapParseWriteOnlyTrue
};

class MapParser {
public:
  MapParser() {};

  void parse(const CellPred& predicate,
             bool arrayOut[MAP_X_CELLS][MAP_Y_CELLS],
             const MapParseWriteRule writeRule = mapParseWriteAlways);
};

//Function object for sorting stl containers by distance to origin
struct IsCloserToOrigin {
public:
  IsCloserToOrigin(const Pos& c, const Engine* engine) :
    c_(c), eng(engine) {
  }
  bool operator()(const Pos& c1, const Pos& c2);
  Pos c_;
  const Engine* eng;
};

//class MapTests {
//public:
//  MapTests(Engine* engine) : eng(engine) {}
//
//  void getActorsPositions(const vector<Actor*>& actors,
//                          vector<Pos>& vectorToFill);
//
//  void makeVisionBlockerArray(
//    const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS],
//    const int MAX_VISION_RANGE = FOV_MAX_RADI_INT);
//
//  void makeMoveBlockerArray(
//    const Actor* const actorMoving,
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void makeMoveBlockerArrayForBodyType(
//    const BodyType_t bodyType,
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void makeShootBlockerFeaturesArray(
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void makeItemBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void makeMoveBlockerArrayFeaturesOnly(
//    const Actor* const actorMoving,
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//    const BodyType_t bodyType,
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void addItemsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void addLivingActorsToBlockerArray(
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void addAllActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void addAdjLivingActorsToBlockerArray(
//    const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void makeWalkBlockingArrayFeaturesOnly(
//    bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]);
//
//  void floodFill(
//    const Pos& origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
//    int values[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit,
//    const Pos& target);
//
//
//  inline bool isPosInsideMap(const Pos& pos) const {
//    if(
//      pos.x < 0 || pos.y < 0 || pos.x >= MAP_X_CELLS || pos.y >= MAP_Y_CELLS) {
//      return false;
//    }
//    return true;
//  }
//
//  inline bool isAreaInsideMap(const Rect& area) {
//    if(
//      area.x0y0.x < 0 || area.x0y0.y < 0 ||
//      area.x1y1.x >= MAP_X_CELLS || area.x1y1.y >= MAP_Y_CELLS) {
//      return false;
//    }
//    return true;
//  }
//
//  inline bool isAreaInsideOther(const Rect& inner, const Rect& outer,
//                                const bool COUNT_EQUAL_AS_INSIDE) const {
//    if(COUNT_EQUAL_AS_INSIDE) {
//      return
//        inner.x0y0.x >= outer.x0y0.x &&
//        inner.x1y1.x <= outer.x1y1.x &&
//        inner.x0y0.y >= outer.x0y0.y &&
//        inner.x1y1.y <= outer.x1y1.y;
//    } else {
//      return
//        inner.x0y0.x > outer.x0y0.x &&
//        inner.x1y1.x < outer.x1y1.x &&
//        inner.x0y0.y > outer.x0y0.y &&
//        inner.x1y1.y < outer.x1y1.y;
//    }
//  }
//
//  inline bool isPosInside(const Pos& pos, const Rect& area) const {
//    return
//      pos.x >= area.x0y0.x &&
//      pos.x <= area.x1y1.x &&
//      pos.y >= area.x0y0.y &&
//      pos.y <= area.x1y1.y;
//  }
//
//  bool isCellNextToPlayer(const Pos& pos,
//                          const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;
//
//  bool isCellsAdj(const Pos& pos1, const Pos& pos2,
//                  const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;
//
//  Pos getClosestPos(const Pos c, const vector<Pos>& positions) const;
//  Actor* getClosestActor(const Pos c, const vector<Actor*>& actors) const;
//
//  void getLine(const Pos& origin, const Pos& target,
//               bool stopAtTarget, int chebTravelLimit,
//               vector<Pos>& posList);
//
//  Actor* getActorAtPos(const Pos pos) const;
//
//private:
//  Engine* eng;
//};

#endif
