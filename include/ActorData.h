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
  cultist, cultistTeslaCannon, cultistSpikeGun,
  cultistPriest,
  rat, ratThing,
  greenSpider, whiteSpider, redSpider, shadowSpider,
  lengSpider,
  miGo,
  flyingPolyp,
  ghoul,
  shadow,
  wolf, fireHound, frostHound, zuul,
  ghost, phantasm, wraith,
  giantBat, byakhee,
  giantMantis, locust,
  mummy, khephren, nitokris,
  deepOne,
  keziahMason, brownJenkin,
  lengElder,
//  lordOfPestilence,
//  lordOfShadows,
//  lordOfSpiders,
//  lordOfSpirits,
  majorClaphamLee, deanHalsey,
  wormMass,
  dustVortex, fireVortex, frostVortex,
  oozeBlack, oozeClear, oozePutrid, oozePoison,
  colourOOSpace,
  chthonian,
  huntingHorror,
  sentryDrone,

  END
};

enum class MonGroupSize {alone, few, group, horde, swarm};

enum class ActorSpeed   {sluggish, slow, normal, fast, fastest, END};

enum ActorErraticFreq
{
  actorErratic_never    = 0,
  actorErratic_rare     = 8,
  actorErratic_somewhat = 25,
  actorErratic_very     = 50
};

enum ActorSize
{
  actorSize_none,
  actorSize_floor,
  actorSize_humanoid,
  actorSize_giant
};

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
  bool intrProps[endOfPropIds];
  int rangedCooldownTurns, spellCooldownTurns;
  bool ai[int(AiId::END)];
  int nrTurnsAwarePlayer;
  int spawnMinDLVL, spawnMaxDLVL;
  ActorSize actorSize;
  bool isHumanoid;
  bool isAutoDescrAllowed;
  std::string deathMessageOverride;
  int nrKills;
  bool canOpenDoors, canBashDoors;
  bool canSeeInDarkness;
  int nrLeftAllowedToSpawn;
  bool isUnique;
  bool isAutoSpawnAllowed;
  std::string spellCastMessage;
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
