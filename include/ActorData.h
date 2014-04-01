#ifndef ACTOR_DATA_H
#define ACTOR_DATA_H

#include <string>
#include <vector>

#include "Converters.h"
#include "AbilityValues.h"
#include "RoomTheme.h"
#include "CommonData.h"
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
  actor_colourOutOfSpace,
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

enum AiBehaviorId {
  aiLooks,
  aiMakesRoomForFriend,
  aiAttacks,
  aiPathsToTargetWhenAware,
  aiMovesTowardTargetWhenVision,
  aiMovesTowardLair,
  aiMovesTowardLeader,
  endOfAiBehaviorId
};

struct ActorData {
public:
  ActorData() {reset();}

  void reset();

  ActorId id;
  string name_a;
  string name_the;
  TileId tile;
  char glyph;
  SDL_Color color;
  MonsterGroupSize groupSize;
  int hp, spi, dmgMelee, dmgRanged;
  ActorSpeed speed;
  AbilityValues abilityVals;
  bool intrProps[endOfPropIds];
  int rangedCooldownTurns, spellCooldownTurns;
  bool ai[endOfAiBehaviorId];
  int nrTurnsAwarePlayer;
  int spawnMinDLVL, spawnMaxDLVL;
  ActorSize actorSize;
  bool isHumanoid;
  bool isAutoDescriptionAllowed;
  string deathMessageOverride;
  int nrOfKills;
  bool canOpenDoors, canBashDoors;
  bool canSeeInDarkness;
  int nrLeftAllowedToSpawn;
  bool isUnique;
  bool isAutoSpawnAllowed;
  string spellCastMessage;
  ActorErraticFreq erraticMovement;
  MonsterShockLevel monsterShockLevel;
  bool isRat, isCanine, isSpider, isUndead, isGhost;
  bool canBeSummoned;
  bool canBleed;
  bool canLeaveCorpse;
  vector<RoomThemeId> nativeRooms;
  string description;
  string aggroTextMonsterSeen;
  string aggroTextMonsterHidden;
  SfxId aggroSfxMonsterSeen;
  SfxId aggroSfxMonsterHidden;
};

class ActorDataHandler {
public:
  ActorDataHandler(Engine& engine) {
    for(unsigned int i = 0; i < endOfActorIds; i++) {
      dataList[i].abilityVals.eng = &engine;
    }
    initDataList();
  }

  ActorData dataList[endOfActorIds];

  void addSaveLines(vector<string>& lines) const;

  void setParamsFromSaveLines(vector<string>& lines);

private:
  void initDataList();
  void addData(ActorData& d);
};

#endif
