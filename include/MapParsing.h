#ifndef MAP_PARSING_H
#define MAP_PARSING_H

#include <vector>

#include "CmnTypes.h"
#include "Config.h"
#include "FeatureData.h"

struct Cell;
class FeatureMob;
class Actor;

namespace CellPred {

class Pred {
public:
  virtual ~Pred() {}
  virtual bool isCheckingCells()          const {return false;}
  virtual bool isCheckingMobFeatures()    const {return false;}
  virtual bool isCheckingActors()         const {return false;}
  virtual bool check(const Cell& c)       const {(void)c; return false;}
  virtual bool check(const FeatureMob& f) const {(void)f; return false;}
  virtual bool check(const Actor& a)      const {(void)a; return false;}
protected:
  Pred() {}
};

class BlocksVision : public Pred {
public:
  BlocksVision() : Pred() {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

class BlocksMoveCmn : public Pred {
public:
  BlocksMoveCmn(bool isActorsBlocking) :
    Pred(), IS_ACTORS_BLOCKING_(isActorsBlocking) {}
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
  BlocksActor(Actor& actor, bool isActorsBlocking);
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool isCheckingActors()         const override {return IS_ACTORS_BLOCKING_;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
  bool check(const Actor& a)      const override;
private:
  const bool IS_ACTORS_BLOCKING_;
  std::vector<PropId> actorsProps_;
};

class BlocksProjectiles : public Pred {
public:
  BlocksProjectiles() : Pred() {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

class LivingActorsAdjToPos : public Pred {
public:
  LivingActorsAdjToPos(const Pos& pos) :
    Pred(), pos_(pos) {}
  bool isCheckingActors()         const override {return true;}
  bool check(const Actor& a)      const override;
  const Pos& pos_;
};

class BlocksItems : public Pred {
public:
  BlocksItems() : Pred() {}
  bool isCheckingCells()          const override {return true;}
  bool isCheckingMobFeatures()    const override {return true;}
  bool check(const Cell& c)       const override;
  bool check(const FeatureMob& f) const override;
};

//class Corridor : public Pred {
//public:
//  Corridor() : Pred() {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

// E.g. ##
//      #.
//      ##
//class Nook : public Pred {
//public:
//  Nook() : Pred() {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

class IsFeature : public Pred {
public:
  IsFeature(const FeatureId id) : Pred(), feature_(id) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  const FeatureId feature_;
};

class IsAnyOfFeatures : public Pred {
public:
  IsAnyOfFeatures(const std::vector<FeatureId>& features) :
    Pred(), features_(features) {}
  IsAnyOfFeatures(const FeatureId id) :
    Pred(), features_(std::vector<FeatureId> {id}) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  std::vector<FeatureId> features_;
};

class AllAdjIsFeature : public Pred {
public:
  AllAdjIsFeature(const FeatureId id) : Pred(), feature_(id) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  const FeatureId feature_;
};

class AllAdjIsAnyOfFeatures : public Pred {
public:
  AllAdjIsAnyOfFeatures(const std::vector<FeatureId>& features) :
    Pred(), features_(features) {}
  AllAdjIsAnyOfFeatures(const FeatureId id) :
    Pred(), features_(std::vector<FeatureId> {id}) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  std::vector<FeatureId> features_;
};

class AllAdjIsNotFeature : public Pred {
public:
  AllAdjIsNotFeature(const FeatureId id) : Pred(), feature_(id) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  const FeatureId feature_;
};

class AllAdjIsNoneOfFeatures : public Pred {
public:
  AllAdjIsNoneOfFeatures(const std::vector<FeatureId>& features) :
    Pred(), features_(features) {}
  AllAdjIsNoneOfFeatures(const FeatureId id) :
    Pred(), features_(std::vector<FeatureId> {id}) {}
  bool isCheckingCells()          const override {return true;}
  bool check(const Cell& c)       const override;
private:
  std::vector<FeatureId> features_;
};

} //CellPred

enum class MapParseWriteRule {always, writeOnlyTrue};

namespace MapParse {

void parse(const CellPred::Pred& predicate, bool out[MAP_W][MAP_H],
           const MapParseWriteRule writeRule = MapParseWriteRule::always);

//Given a map array of booleans, this will fill a second map array of boolens
//where the cells are set to true if they are within the specified distance
//interval of the first array.
//This can be used for example to find all cells up to 3 steps from a wall.
void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H],
                                bool out[MAP_W][MAP_H],
                                const Range& distInterval);

bool isValInArea(const Rect& area, const bool in[MAP_W][MAP_H],
                 const bool VAL = true);

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]);

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST,
            const bool VAL_TO_EXPAND = true);

} //MapParse

//Function object for sorting STL containers by distance to a position
struct IsCloserToPos {
public:
  IsCloserToPos(const Pos& p) : p_(p) {}
  bool operator()(const Pos& p1, const Pos& p2);
  Pos p_;
};

namespace FloodFill {

void run(const Pos& p0, bool blocked[MAP_W][MAP_H],
         int out[MAP_W][MAP_H], int travelLimit, const Pos& p1,
         const bool ALLOW_DIAGONAL);

} //FloodFill

namespace PathFind {

//Note: The resulting path does not include the origin
void run(const Pos& p0, const Pos& p1, bool blocked[MAP_W][MAP_H],
         std::vector<Pos>& out, const bool ALLOW_DIAGONAL = true);

} //PathFind

#endif
