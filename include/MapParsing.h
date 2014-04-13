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

class Pred {
public:
  virtual bool isCheckingCells()          const {return false;}
  virtual bool isCheckingMobFeatures()    const {return false;}
  virtual bool isCheckingActors()         const {return false;}
  virtual bool check(const Cell& c)       const {(void)c; return false;}
  virtual bool check(const FeatureMob& f) const {(void)f; return false;}
  virtual bool check(const Actor& a)      const {(void)a; return false;}
  const Engine& eng;
protected:
  Pred(Engine& engine) : eng(engine) {}
//  bool canWalkAt(const int X, const int Y) const;
};

class BlocksVision : public Pred {
public:
  BlocksVision(Engine& engine) : Pred(engine) {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

class BlocksMoveCmn : public Pred {
public:
  BlocksMoveCmn(bool isActorsBlocking, Engine& engine) :
    Pred(engine), IS_ACTORS_BLOCKING_(isActorsBlocking) {}

  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool isCheckingActors()         const override {return IS_ACTORS_BLOCKING_;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
  bool check(const Actor& a)      const override;
private:
  const bool IS_ACTORS_BLOCKING_;
};

class BlocksActor : public Pred {
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

class BlocksProjectiles : public Pred {
public:
  BlocksProjectiles(Engine& engine) : Pred(engine) {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

class LivingActorsAdjToPos : public Pred {
public:
  LivingActorsAdjToPos(const Pos& pos, Engine& engine) :
    Pred(engine), pos_(pos) {}
  bool isCheckingActors()         const override {return true;}
  bool check(const Actor& a)      const override;
  const Pos& pos_;
};

class BlocksItems : public Pred {
public:
  BlocksItems(Engine& engine) : Pred(engine) {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

//class Corridor : public Pred {
//public:
//  Corridor(Engine& engine) : Pred(engine) {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

// E.g. ##
//      #.
//      ##
//class Nook : public Pred {
//public:
//  Nook(Engine& engine) : Pred(engine) {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

class IsAnyOfFeatures : public Pred {
public:
  IsAnyOfFeatures(Engine& engine, const vector<FeatureId>& features) :
    Pred(engine), features_(features) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  vector<FeatureId> features_;
};

class AllAdjIsAnyOfFeatures : public Pred {
public:
  AllAdjIsAnyOfFeatures(Engine& engine, const vector<FeatureId>& features) :
    Pred(engine), features_(features) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  vector<FeatureId> features_;
};

} //CellPred

enum class MapParseWriteRule {always, writeOnlyTrue};

namespace MapParse {

void parse(const CellPred::Pred& predicate, bool arrayOut[MAP_W][MAP_H],
           const MapParseWriteRule writeRule = MapParseWriteRule::always);

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
         int values[MAP_W][MAP_H], int travelLimit, const Pos& target);

} //FloodFill

namespace PathFind {

void run(const Pos& origin, const Pos& target, bool blockers[MAP_W][MAP_H],
         vector<Pos>& vectorRef);

} //PathFind

#endif
