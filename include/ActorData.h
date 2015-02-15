#ifndef ACTOR_DATA_H
#define ACTOR_DATA_H

#include <string>
#include <vector>

#include "Converters.h"
#include "AbilityValues.h"
#include "CmnData.h"
#include "Properties.h"
#include "Audio.h"
#include "Room.h"

enum class ActorId
{
    player,
    zombie, zombieAxe, bloatedZombie,
    cultist, cultistElectric, cultistSpikeGun,
    cultistPriest,
    rat, ratThing,
    greenSpider, whiteSpider, redSpider, shadowSpider,
    lengSpider,
    miGo, miGoCommander,
    flyingPolyp, greaterPolyp,
    ghoul,
    shadow,
    wolf, fireHound, frostHound, zuul,
    ghost, phantasm, wraith,
    giantBat, byakhee,
    giantMantis, locust,
    mummy, crocHeadMummy, khephren, nitokris,
    deepOne,
    ape,
    keziahMason, brownJenkin,
    lengElder,
    majorClaphamLee, deanHalsey,
    wormMass,
    dustVortex, fireVortex, frostVortex,
    oozeBlack, oozeClear, oozePutrid, oozePoison,
    colourOOSpace,
    chthonian,
    huntingHorror,
    sentryDrone,
    mold,
    gasSpore,
    theHighPriest, theHighPriestCpy,

    END
};

enum class MonGroupSize
{
    alone,
    few,
    group,
    horde,
    swarm
};

enum class ActorSpeed
{
    sluggish,
    slow,
    normal,
    fast,
    fastest,
    END
};

enum class ActorErraticFreq
{
    never    = 0,
    rare     = 8,
    somewhat = 25,
    very     = 50
};

enum class ActorSize {none, floor, humanoid, giant};

enum class MonShockLvl {none, unsettling, scary, terrifying, mindShattering, END};

enum class AiId
{
    looks,
    makesRoomForFriend,
    attacks,
    pathsToTgtWhenAware,
    movesToTgtWhenLos,
    movesToLair,
    movesToLeader,
    movesToRandomWhenUnaware,
    END
};

struct ActorDataT
{
    ActorDataT() {reset();}

    void reset();

    ActorId id;
    std::string nameA;
    std::string nameThe;
    std::string corpseNameA;
    std::string corpseNameThe;
    TileId tile;
    char glyph;
    Clr color;
    MonGroupSize groupSize;
    int hp, spi, dmgMelee, dmgRanged;
    ActorSpeed speed;
    AbilityVals abilityVals;
    bool intrProps[size_t(PropId::END)];
    int rangedCooldownTurns, spellCooldownTurns;
    bool ai[int(AiId::END)];
    int nrTurnsAware;
    int spawnMinDLVL, spawnMaxDLVL;
    ActorSize actorSize;
    bool isHumanoid;
    bool isAutoDescrAllowed;
    std::string deathMsgOverride;
    int nrKills;
    bool canOpenDoors, canBashDoors;
    bool canSeeInDarkness;
    //NOTE: Knockback may be prevented even if this is false, e.g. if monster is ethereal
    bool preventKnockback;
    int nrLeftAllowedToSpawn;
    bool isUnique;
    bool isAutoSpawnAllowed;
    std::string spellCastMsg;
    ActorErraticFreq erraticMovePct;
    MonShockLvl monShockLvl;
    bool isRat, isCanine, isSpider, isUndead, isGhost;
    bool canBeSummoned;
    bool canBleed;
    bool canLeaveCorpse;
    std::vector<RoomType> nativeRooms;
    std::string descr;
    std::string aggroTextMonSeen;
    std::string aggroTextMonHidden;
    SfxId aggroSfxMonSeen;
    SfxId aggroSfxMonHidden;
};

namespace ActorData
{

extern ActorDataT data[int(ActorId::END)];

void init();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

} //ActorData

#endif
