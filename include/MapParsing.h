#ifndef MAP_PARSING_H
#define MAP_PARSING_H

#include <vector>

#include "CommonTypes.h"
#include "Config.h"
#include "FeatureData.h"

class Engine;
struct Cell;
class FeatureMob;
class Actor;

namespace CellPred {

class CellPred {
public:
  virtual bool isCheckingCells()          const {return false;}
  virtual bool isCheckingMobFeatures()    const {return false;}
  virtual bool isCheckingActors()         const {return false;}
  virtual bool check(const Cell& c)       const {(void)c; return false;}
  virtual bool check(const FeatureMob& f) const {(void)f; return false;}
  virtual bool check(const Actor& a)      const {(void)a; return false;}
  const Engine& eng;
protected:
  CellPred(Engine& engine) : eng(engine) {}
//  bool canWalkAt(const int X, const int Y) const;
};

class BlocksVision : public CellPred {
public:
  BlocksVision(Engine& engine) : CellPred(engine) {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

class BlocksMoveCmn : public CellPred {
public:
  BlocksMoveCmn(bool isActorsBlocking, Engine& engine) :
    CellPred(engine), IS_ACTORS_BLOCKING_(isActorsBlocking) {}

  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool isCheckingActors()         const override {return IS_ACTORS_BLOCKING_;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
  bool check(const Actor& a)      const override;
private:
  const bool IS_ACTORS_BLOCKING_;
};

class BlocksActor : public CellPred {
public:
  BlocksActor(Actor& actor, bool isActorsBlocking, Engine& engine);

  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool isCheckingActors()         const override {return IS_ACTORS_BLOCKING_;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
  bool check(const Actor& a)      const override;
private:
  const bool IS_ACTORS_BLOCKING_;
  vector<PropId> actorsProps_;
};

class BlocksProjectiles : public CellPred {
public:
  BlocksProjectiles(Engine& engine) : CellPred(engine) {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

class LivingActorsAdjToPos : public CellPred {
public:
  LivingActorsAdjToPos(const Pos& pos, Engine& engine) :
    CellPred(engine), pos_(pos) {}
  bool isCheckingActors()         const override {return true;}
  bool check(const Actor& a)      const override;
  const Pos& pos_;
};

class BlocksItems : public CellPred {
public:
  BlocksItems(Engine& engine) : CellPred(engine) {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

//class Corridor : public CellPred {
//public:
//  Corridor(Engine& engine) : CellPred(engine) {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

// E.g. ##
//      #.
//      ##
//class Nook : public CellPred {
//public:
//  Nook(Engine& engine) : CellPred(engine) {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

class IsAnyOfFeatures : public CellPred {
public:
  IsAnyOfFeatures(Engine& engine, const vector<FeatureId>& features) :
    CellPred(engine), features_(features) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  vector<FeatureId> features_;
};

class AllAdjIsAnyOfFeatures : public CellPred {
public:
  AllAdjIsAnyOfFeatures(Engine& engine, const vector<FeatureId>& features) :
    CellPred(engine), features_(features) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  vector<FeatureId> features_;
};

} //CellPred

enum MapParseWriteRule {
  mapParseWriteAlways, mapParseWriteOnlyTrue
};

namespace MapParse {

void parse(const CellPred::CellPred& predicate, bool arrayOut[MAP_W][MAP_H],
           const MapParseWriteRule writeRule = mapParseWriteAlways);

void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H],
                                bool out[MAP_W][MAP_H],
                                const Range& distIntervall);

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]);

} //MapParse

//Function object for sorting STL containers by distance to origin
struct IsCloserToOrigin {
public:
  IsCloserToOrigin(const Pos& c, const Engine& engine) : c_(c), eng(engine) {}
  bool operator()(const Pos& c1, const Pos& c2);
  Pos c_;
  const Engine& eng;
};

namespace FloodFill {

void run(const Pos& origin, bool blockers[MAP_W][MAP_H],
         int values[MAP_W][MAP_H], int travelLimit, const Pos& target,
         Engine& eng);

} //FloodFill

namespace PathFind {

void run(const Pos& origin, const Pos& target, bool blockers[MAP_W][MAP_H],
         vector<Pos>& vectorRef, Engine& eng);

} //PathFind

#endif
