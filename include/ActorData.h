#ifndef ACTOR_DATA_H
#define ACTOR_DATA_H

#include <string>
#include <vector>

#include "Converters.h"
#include "AbilityValues.h"
#include "RoomTheme.h"
#include "CommonData.h"
#include "Properties.h"
#include "AudioIds.h"

enum ActorId_t {
  actor_empty, //TODO Is it really necessary to have an "empty"(?) actor?
  actor_player,
  actor_zombie, actor_zombieAxe, actor_bloatedZombie,
  actor_cultist, actor_cultistTeslaCannon, actor_cultistSpikeGun,
  actor_cultistPriest,
  actor_rat, actor_ratThing,
  actor_greenSpider, actor_whiteSpider, actor_redSpider, actor_shadowSpider, actor_lengSpider,
  actor_miGo,
  actor_ghoul, actor_shadow,
  actor_wolf, actor_fireHound, actor_frostHound,
  actor_ghost, actor_phantasm, actor_wraith,
  actor_giantBat, actor_byakhee,
  actor_giantMantis, actor_giantLocust,
  actor_mummy, actor_khephren, actor_nitokris,
  actor_deepOne,
  actor_keziahMason, actor_brownJenkin,
//  actor_lordOfPestilence, actor_lordOfShadows, actor_lordOfSpiders, actor_lordOfSpirits,
  actor_majorClaphamLee, actor_deanHalsey,
  actor_wormMass,
  actor_dustVortex, actor_fireVortex, actor_frostVortex,
  actor_oozeBlack, actor_oozeClear, actor_oozePutrid, actor_oozePoison, actor_colourOutOfSpace,
  actor_huntingHorror,

  endOfActorIds
};

enum MonsterGroupSize_t {
  monsterGroupSize_alone,
  monsterGroupSize_few,
  monsterGroupSize_group,
  monsterGroupSize_horde,
  monsterGroupSize_swarm
};

enum ActorSpeed_t {
  actorSpeed_sluggish = 0,
  actorSpeed_slow     = 1,
  actorSpeed_normal   = 2,
  actorSpeed_fast     = 3,
  actorSpeed_fastest  = 4
};

enum ActorErratic_t {
  actorErratic_never    = 0,
  actorErratic_rare     = 8,
  actorErratic_somewhat = 25,
  actorErratic_very     = 50
};

enum ActorSizes_t {
  actorSize_none,
  actorSize_floor,
  actorSize_humanoid,
  actorSize_giant
};

enum MonsterShockLevel {
  monsterShockLevel_none            = 0,
  monsterShockLevel_unsettling      = 1,
  monsterShockLevel_scary           = 2,
  monsterShockLevel_terrifying      = 3,
  monsterShockLevel_mindShattering  = 4
};

enum AiBehavior_t {
  aiLooks,
  aiMakesRoomForFriend,
  aiAttacks,
  aiPathsToTargetWhenAware,
  aiMovesTowardTargetWhenVision,
  aiMovesTowardLair,
  aiMovesTowardLeader,
  endOfAiBehavior
};

struct ActorData {
public:
  ActorData() {reset();}

  void reset();

  ActorId_t id;
  BodyType_t bodyType;
  string name_a;
  string name_the;
  Tile_t tile;
  char glyph;
  SDL_Color color;
  MonsterGroupSize_t groupSize;
  int hp, spi, dmgMelee, dmgRanged;
  ActorSpeed_t speed;
  AbilityValues abilityVals;
  bool intrProps[endOfPropIds];
  int rangedCooldownTurns, spellCooldownTurns;
  bool ai[endOfAiBehavior];
  int nrTurnsAwarePlayer;
  int spawnMinDLVL, spawnMaxDLVL;
  ActorSizes_t actorSize;
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
  ActorErratic_t erraticMovement;
  MonsterShockLevel monsterShockLevel;
  bool isRat, isCanine, isSpider, isUndead, isGhost;
  bool canBeSummoned;
  bool canBleed;
  bool canDodge;
  vector<RoomTheme_t> nativeRooms;
  string description;
  string aggroTextMonsterSeen;
  string aggroTextMonsterHidden;
  Sfx_t aggroSfxMonsterSeen;
  Sfx_t aggroSfxMonsterHidden;
};

class ActorDataHandler {
public:
  ActorDataHandler(Engine& engine) : eng(engine) {
    for(unsigned int i = 0; i < endOfActorIds; i++) {
      dataList[i].abilityVals.eng = &engine;
    }
    initDataList();
  }

  ActorData dataList[endOfActorIds];

  void addSaveLines(vector<string>& lines) const {
    for(unsigned int i = 0; i < endOfActorIds; i++) {
      lines.push_back(toString(dataList[i].nrLeftAllowedToSpawn));
      lines.push_back(toString(dataList[i].nrOfKills));
    }
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    for(unsigned int i = 0; i < endOfActorIds; i++) {
      dataList[i].nrLeftAllowedToSpawn = toInt(lines.front());
      lines.erase(lines.begin());
      dataList[i].nrOfKills = toInt(lines.front());
      lines.erase(lines.begin());
    }
  }

private:
  void initDataList();
  void addData(ActorData& d);

  Engine& eng;
};

#endif
