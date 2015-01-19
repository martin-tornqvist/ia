#ifndef MAP_PARSING_H
#define MAP_PARSING_H

#include <vector>

#include "CmnTypes.h"
#include "Config.h"
#include "FeatureData.h"

struct Cell;
class Mob;
class Actor;

namespace CellCheck
{

class Check
{
public:
    virtual ~Check() {}
    virtual bool isCheckingCells()          const {return false;}
    virtual bool isCheckingMobs()           const {return false;}
    virtual bool isCheckingActors()         const {return false;}
    virtual bool check(const Cell& c)       const {(void)c; return false;}
    virtual bool check(const Mob& f)        const {(void)f; return false;}
    virtual bool check(const Actor& a)      const {(void)a; return false;}
protected:
    Check() {}
};

class BlocksLos : public Check
{
public:
    BlocksLos() : Check() {}
    bool isCheckingCells()          const override {return true;}
    bool isCheckingMobs()           const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class BlocksMoveCmn : public Check
{
public:
    BlocksMoveCmn(bool isActorsBlocking) :
        Check(), IS_ACTORS_BLOCKING_(isActorsBlocking) {}
    bool isCheckingCells()          const override {return true;}
    bool isCheckingMobs()           const override {return true;}
    bool isCheckingActors()         const override {return IS_ACTORS_BLOCKING_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool IS_ACTORS_BLOCKING_;
};

class BlocksActor : public Check
{
public:
    BlocksActor(Actor& actor, bool isActorsBlocking);
    bool isCheckingCells()          const override {return true;}
    bool isCheckingMobs()           const override {return true;}
    bool isCheckingActors()         const override {return IS_ACTORS_BLOCKING_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool IS_ACTORS_BLOCKING_;
    bool actorsProps_[size_t(PropId::END)];
};

class BlocksProjectiles : public Check
{
public:
    BlocksProjectiles() : Check() {}
    bool isCheckingCells()          const override {return true;}
    bool isCheckingMobs()           const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class LivingActorsAdjToPos : public Check
{
public:
    LivingActorsAdjToPos(const Pos& pos) :
        Check(), pos_(pos) {}
    bool isCheckingActors()         const override {return true;}
    bool check(const Actor& a)      const override;
    const Pos& pos_;
};

class BlocksItems : public Check
{
public:
    BlocksItems() : Check() {}
    bool isCheckingCells()          const override {return true;}
    bool isCheckingMobs()           const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

//class Corridor : public Check {
//public:
//  Corridor() : Check() {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

// E.g. ##
//      #.
//      ##
//class Nook : public Check {
//public:
//  Nook() : Check() {}
//  bool isCheckingCells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

class IsFeature : public Check
{
public:
    IsFeature(const FeatureId id) : Check(), feature_(id) {}
    bool isCheckingCells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const FeatureId feature_;
};

class IsAnyOfFeatures : public Check
{
public:
    IsAnyOfFeatures(const std::vector<FeatureId>& features) :
        Check(), features_(features) {}

    IsAnyOfFeatures(const FeatureId id) :
        Check(), features_(std::vector<FeatureId> {id}) {}

    bool isCheckingCells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<FeatureId> features_;
};

class AllAdjIsFeature : public Check
{
public:
    AllAdjIsFeature(const FeatureId id) : Check(), feature_(id) {}
    bool isCheckingCells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const FeatureId feature_;
};

class AllAdjIsAnyOfFeatures : public Check
{
public:
    AllAdjIsAnyOfFeatures(const std::vector<FeatureId>& features) :
        Check(), features_(features) {}

    AllAdjIsAnyOfFeatures(const FeatureId id) :
        Check(), features_(std::vector<FeatureId> {id}) {}

    bool isCheckingCells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<FeatureId> features_;
};

class AllAdjIsNotFeature : public Check
{
public:
    AllAdjIsNotFeature(const FeatureId id) : Check(), feature_(id) {}
    bool isCheckingCells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const FeatureId feature_;
};

class AllAdjIsNoneOfFeatures : public Check
{
public:
    AllAdjIsNoneOfFeatures(const std::vector<FeatureId>& features) :
        Check(), features_(features) {}

    AllAdjIsNoneOfFeatures(const FeatureId id) :
        Check(), features_(std::vector<FeatureId> {id}) {}

    bool isCheckingCells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<FeatureId> features_;
};

} //CellCheck

//Note: If append mode is used, the caller is responsible for initializing the array
//(typically with a previous parse call, with write rule set to "overwrite")
enum class MapParseMode {overwrite, append};

namespace MapParse
{

extern const Rect mapRect;

void run(const CellCheck::Check& check, bool out[MAP_W][MAP_H],
         const MapParseMode writeRule = MapParseMode::overwrite,
         const Rect& areaToCheckCells = mapRect);

//Given a map array of booleans, this will fill a second map array of boolens
//where the cells are set to true if they are within the specified distance
//interval of the first array.
//This can be used for example to find all cells up to 3 steps from a wall.
void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
                                const Range& distInterval);

bool isValInArea(const Rect& area, const bool in[MAP_W][MAP_H], const bool VAL = true);

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]);

//Optimized for expanding with a distance of one
void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
            const Rect& areaAllowedToModify = Rect(0, 0, MAP_W, MAP_H));

//Slower version that can expand any distance
void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST);

bool isMapConnected(const bool blocked[MAP_W][MAP_H]);

} //MapParse

//Function object for sorting STL containers by distance to a position
struct IsCloserToPos
{
public:
    IsCloserToPos(const Pos& p) : p_(p) {}
    bool operator()(const Pos& p1, const Pos& p2);
    Pos p_;
};

namespace FloodFill
{

void run(const Pos& p0, const bool blocked[MAP_W][MAP_H], int out[MAP_W][MAP_H],
         int travelLmt, const Pos& p1, const bool ALLOW_DIAGONAL);

} //FloodFill

namespace PathFind
{

//Note: The path goes from target to origin, not including the origin.
//---------------------------------------------------------------------------------------
//RANDOMIZE_STEP_CHOICES: When true, for each step if there are multiple valid (nearer)
//                        choices, pick one at random. Otherwise iterate over a predefined
//                        list of offsets until a valid step is found. The second way is
//                        more optimized and is the default behavior (best for e.g. AI),
//                        while the randomized method can produces nicer results in some
//                        cases (e.g. corridors).
void run(const Pos& p0, const Pos& p1, bool blocked[MAP_W][MAP_H],
         std::vector<Pos>& out, const bool ALLOW_DIAGONAL = true,
         const bool RANDOMIZE_STEP_CHOICES = false);

} //PathFind

#endif
