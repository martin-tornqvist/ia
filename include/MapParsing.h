#ifndef MAP_PARSING_H
#define MAP_PARSING_H

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
  virtual bool isCheckingCells()          const = 0;
  virtual bool isCheckingMobFeatures()    const = 0;
  virtual bool isCheckingActors()         const = 0;
  virtual bool check(const Cell& c)       const = 0;
  virtual bool check(const FeatureMob& f) const = 0;
  virtual bool check(const Actor& a)      const = 0;
  const Engine* const eng;
protected:
  CellPred(Engine* engine) : eng(engine) {}
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
  CellPredBlocksBodyType(BodyType_t bodyType, bool isActorsBlocking,
                         Engine* engine) :
    CellPred(engine), bodyType_(bodyType),
    IS_ACTORS_BLOCKING_(isActorsBlocking) {}
  bool isCheckingCells()          const {return true;}
  bool isCheckingMobFeatures()    const {return true;}
  bool isCheckingActors()         const {return IS_ACTORS_BLOCKING_;}
  bool check(const Cell& c)       const;
  bool check(const FeatureMob& f) const;
  bool check(const Actor& a)      const;
  const BodyType_t bodyType_;
  const bool IS_ACTORS_BLOCKING_;
};

class CellPredBlocksProjectiles : public CellPred {
public:
  CellPredBlocksProjectiles(Engine* engine) : CellPred(engine) {}
  bool isCheckingCells()          const {return true;}
  bool isCheckingMobFeatures()    const {return true;}
  bool isCheckingActors()         const {return false;}
  bool check(const Cell& c)       const;
  bool check(const FeatureMob& f) const;
  bool check(const Actor& a)      const {(void)a; return false;}
};

class CellPredLivingActorsAdjToPos : public CellPred {
public:
  CellPredLivingActorsAdjToPos(const Pos& pos, Engine* engine) :
    CellPred(engine), pos_(pos) {}
  bool isCheckingCells()          const {return false;}
  bool isCheckingMobFeatures()    const {return false;}
  bool isCheckingActors()         const {return true;}
  bool check(const Cell& c)       const {(void)c; return false;}
  bool check(const FeatureMob& f) const {(void)f; return false;}
  bool check(const Actor& a)      const;
  const Pos& pos_;
};

enum MapParseWriteRule {
  mapParseWriteAlways,
  mapParseWriteOnlyFalse,
  mapParseWriteOnlyTrue
};

class MapParser {
public:
  MapParser() {};

  static void parse(const CellPred& predicate,
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
//
//  bool isCellNextToPlayer(const Pos& pos,
//                          const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;
//
//
//
//
//
//private:
//  Engine* eng;
//};

#endif
