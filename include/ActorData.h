#ifndef ACTOR_DATA_H
#define ACTOR_DATA_H

#include <string>
#include <vector>

#include "Converters.h"
#include "AbilityValues.h"
#include "RoomTheme.h"
#include "CmnData.h"
#include "Properties.h"
#include "Audio.h"

enum ActorId {
  actor_empty, //TODO Is it really necessary to have an "empty"(?) actor?
  actor_player,
  actor_zombie, actor_zombieAxe, actor_bloatedZombie,
  actor_cultist, actor_cultistTeslaCannon, actor_cultistSpikeGun,
  actor_cultistPriest,
  actor_rat, actor_ratThing,
  actor_greenSpider, actor_whiteSpider, actor_redSpider, actor_shadowSpider,
  actor_lengSpider,
  actor_miGo,
  actor_flyingPolyp,
  actor_ghoul,
  actor_shadow,
  actor_wolf, actor_fireHound, actor_frostHound, actor_zuul,
  actor_ghost, actor_phantasm, actor_wraith,
  actor_giantBat, actor_byakhee,
  actor_giantMantis, actor_giantLocust,
  actor_mummy, actor_khephren, actor_nitokris,
  actor_deepOne,
  actor_keziahMason, actor_brownJenkin,
//  actor_lordOfPestilence,
//  actor_lordOfShadows,
//  actor_lordOfSpiders,
//  actor_lordOfSpirits,
  actor_majorClaphamLee, actor_deanHalsey,
  actor_wormMass,
  actor_dustVortex, actor_fireVortex, actor_frostVortex,
  actor_oozeBlack, actor_oozeClear, actor_oozePutrid, actor_oozePoison,
  actor_colourOOSpace,
  actor_chthonian,
  actor_huntingHorror,

  endOfActorIds
};

enum MonsterGroupSize {
  monsterGroupSize_alone,
  monsterGroupSizeFew,
  monsterGroupSizeGroup,
  monsterGroupSizeHorde,
  monsterGroupSizeSwarm
};

enum class ActorSpeed {
  sluggish, slow, normal, fast, fastest, endOfActorSpeed
};

enum ActorErraticFreq {
  actorErratic_never    = 0,
  actorErratic_rare     = 8,
  actorErratic_somewhat = 25,
  actorErratic_very     = 50
};

enum ActorSize {
  actorSize_none,
  actorSize_floor,
  actorSize_humanoid,
  actorSize_giant
};

enum class MonsterShockLevel {
  none, unsettling, scary, terrifying, mindShattering, endOfMonsterShockLevel,
};

enum class AiId {
  looks,
  makesRoomForFriend,
  attacks,
  pathsToTargetWhenAware,
  movesTowardTargetWhenVision,
  movesTowardLair,
  movesTowardLeader,
  endOfAi
};

struct ActorDataT {
  ActorDataT() {reset();}

  void reset();

  ActorId id;
  std::string name_a;
  std::string name_the;
  TileId tile;
  char glyph;
  SDL_Color color;
  MonsterGroupSize groupSize;
  int hp, spi, dmgMelee, dmgRanged;
  ActorSpeed speed;
  AbilityVals abilityVals;
  bool intrProps[endOfPropIds];
  int rangedCooldownTurns, spellCooldownTurns;
  bool ai[int(AiId::endOfAi)];
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
  ActorErraticFreq erraticMovement;
  MonsterShockLevel monsterShockLevel;
  bool isRat, isCanine, isSpider, isUndead, isGhost;
  bool canBeSummoned;
  bool canBleed;
  bool canLeaveCorpse;
  std::vector<RoomThemeId> nativeRooms;
  std::string descr;
  std::string aggroTextMonsterSeen;
  std::string aggroTextMonsterHidden;
  SfxId aggroSfxMonsterSeen;
  SfxId aggroSfxMonsterHidden;
};

namespace ActorData {

ActorDataT dataList[endOfActorIds];

void init();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

} //ActorData

#endif
