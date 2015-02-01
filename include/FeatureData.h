#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include "Art.h"
#include "CmnData.h"
#include "Room.h"
#include "MapPatterns.h"
#include "Properties.h"

enum class FeatureId
{
    floor,
    bridge,
    wall,
    tree,
    grass,
    bush,
    stairs,
    lever,
    brazier,
    gravestone,
    tomb,
    churchBench,
    altar,
    carpet,
    rubbleHigh,
    rubbleLow,
    bones,
    statue,
    cocoon,
    chest,
    cabinet,
    fountain,
//  barrel,
    pillar,
    monolith,
    stalagmite,
    chasm,
    liquidShallow,
    liquidDeep,
    door,
    litDynamite,
    litFlare,
    trap,
//  pit,
    smoke,
    eventWallCrumble,
    eventRatsInTheWallsDiscovery,

    END
};

struct FeatureRoomSpawnRules
{
public:
    FeatureRoomSpawnRules();

    void reset();

    void set(const int MAX_NR_IN_ROOM, const Range& dlvlsAllowed,
             const PlacementRule placementRule, std::initializer_list<RoomType> roomTypes);

    bool          isBelongingToRoomType(const RoomType type)  const;
    PlacementRule getPlacementRule()                          const;
    int           getMaxNrInRoom()                            const;
    Range         getDlvlsAllowed()                           const;

private:
    int                   maxNrInRoom_;
    Range                 dlvlsAllowed_;
    PlacementRule         placementRule_;
    std::vector<RoomType> roomTypesNative_;
};

class Actor;

class MoveRules
{
public:
    MoveRules() {reset();}

    ~MoveRules() {}

    void reset()
    {
        canMoveCmn_ = false;

        for (int i = 0; i < int(PropId::END); ++i)
        {
            canMoveIfHaveProp_[i] = false;
        }
    }

    void setPropCanMove(const PropId id) {canMoveIfHaveProp_[int(id)] = true;}
    void setCanMoveCmn() {canMoveCmn_ = true;}

    bool canMoveCmn() const {return canMoveCmn_;}

    bool canMove(const bool actorPropIds[size_t(PropId::END)]) const;

private:
    bool canMoveCmn_;
    bool canMoveIfHaveProp_[size_t(PropId::END)];
};

class Feature;

struct FeatureDataT
{
    std::function<Feature*(const Pos& p)> mkObj;
    FeatureId id;
    char glyph;
    TileId tile;
    MoveRules moveRules;
    bool isSoundPassable;
    bool isProjectilePassable;
    bool isLosPassable;
    bool isSmokePassable;
    bool canHaveBlood;
    bool canHaveGore;
    bool canHaveCorpse;
    bool canHaveRigid;
    bool canHaveItem;
    bool isBottomless;
    Matl matlType;
    std::string msgOnPlayerBlocked;
    std::string msgOnPlayerBlockedBlind;
    int dodgeModifier;
    int shockWhenAdjacent;
    FeatureRoomSpawnRules roomSpawnRules;
};

namespace FeatureData
{

void init();

const FeatureDataT& getData(const FeatureId id);

} //FeatureData

#endif
