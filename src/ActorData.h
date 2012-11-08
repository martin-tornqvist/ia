#ifndef ACTOR_DATA_H
#define ACTOR_DATA_H

#include <string>
#include <vector>

#include "Phrases.h"
#include "Converters.h"
#include "AbilityValues.h"
#include "RoomTheme.h"
#include "ConstTypes.h"

enum ActorDevNames_t {
  actor_empty,
  actor_player,
  actor_zombie, actor_zombieAxe, actor_bloatedZombie,
  actor_cultistShotgun, actor_cultistPistol, actor_cultistMachineGun, actor_cultistTeslaCannon, actor_cultistSpikeGun,
  actor_cultistPriest,
  actor_rat, actor_ratThing,
  actor_greenSpider, actor_whiteSpider, actor_blackSpider, actor_lengSpider,
  actor_miGo,
  actor_ghoul, actor_shadow,
  actor_fireHound, actor_wolf,
  actor_ghost, actor_phantasm, actor_wraith,
  actor_giantBat, actor_byakhee,
  actor_mummy, actor_khephren, actor_nitokris,
  actor_deepOne,
  actor_keziahMason, actor_brownJenkin,
  actor_lordOfPestilence, actor_lordOfShadows, actor_lordOfSpiders, actor_lordOfSpirits,
  actor_majorClaphamLee, actor_deanHalsey,
  actor_wormMass,
  actor_fireVampire,
  actor_oozeGray, actor_oozeClear, actor_oozePutrid,
  actor_huntingHorror,

  endOfActorDevNames
};

enum ShockValues_t
{
  shockValue_none,
  shockValue_mild,
  shockValue_some,
  shockValue_heavy
};

enum ActorSpeed_t
{
  actorSpeed_sluggish,
  actorSpeed_slow,
  actorSpeed_normal,
  actorSpeed_fast,
  actorSpeed_fastest
};

enum ActorErratic_t
{
  actorErratic_never = 0,
  actorErratic_rare = 8,
  actorErratic_somewhat = 25,
  actorErratic_very = 50
};

enum ActorSizes_t {actorSize_none, actorSize_floor, actorSize_humanoid, actorSize_giant};

struct AiBehavior {
public:
  AiBehavior() : looks(false), listens(false), respondsWithPhrase(false),
    makesRoomForFriend(false), attemptsAttack(false), pathsToTargetWhenAware(false),
    movesTowardTargetWhenVision(false), movesTowardLair(false),
    movesTowardLeader(false) {}

  bool looks, listens, respondsWithPhrase, makesRoomForFriend, attemptsAttack,
  pathsToTargetWhenAware, movesTowardTargetWhenVision, movesTowardLair,
  movesTowardLeader;
};

struct ActorDefinition {
public:
  ActorDefinition() {
    reset();
  }

  void reset();

  ActorDevNames_t devName;
  ActorSpeed_t speed;
  int nrTurnsAttackDisablesMelee;
  int nrTurnsAttackDisablesRanged;
  MoveType_t moveType;
  AiBehavior aiBehavior;
  int hpMax;
  string name_a;
  string name_the;
  AbilityValues abilityValues;
  PhraseSets_t phraseSet;
  char glyph;
  SDL_Color color;
  Tile_t tile;
  SpawnRate_t spawnRate;
  bool isAllowedToSpawnAfterMapCreation;
  int spawnStandardMinLevel;
  int spawnStandardMaxLevel;
  int monsterLvl;
  int chanceToSpawnExtra;
  ActorSizes_t actorSize;
  bool isHumanoid;
  string description;
  bool allowAutoDescription;
  string deathMessageOverride;
  int nrOfKills;
  bool canOpenDoors;
  bool canBashDoors;
  int nrTurnsAwarePlayer;
  int nrLeftAllowedToSpawn;
  bool unique;
  string spellCastMessage;
  int spellCooldown;
  ActorErratic_t erraticMovement;
  ShockValues_t shockValue;
  bool isRat, isCanine, isSpider;
  bool isUndead;
  bool canBeSummoned;
  bool canBleed;
  bool canDodge;
  vector<RoomTheme_t> nativeRooms;
};

class ActorData {
public:
  ActorData(Engine* engine) : eng(engine) {
    for(unsigned int i = 0; i < endOfActorDevNames; i++) {
      actorDefinitions[i].abilityValues.eng = engine;
    }
    defineAllActors();
  }

  void defineAllActors();

  ActorDefinition actorDefinitions[endOfActorDevNames];

  void addSaveLines(vector<string>& lines) const {
    for(unsigned int i = 0; i < endOfActorDevNames; i++) {
      lines.push_back(intToString(actorDefinitions[i].nrLeftAllowedToSpawn));
      lines.push_back(intToString(actorDefinitions[i].nrOfKills));
    }
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    for(unsigned int i = 0; i < endOfActorDevNames; i++) {
      actorDefinitions[i].nrLeftAllowedToSpawn = stringToInt(lines.front());
      lines.erase(lines.begin());
      actorDefinitions[i].nrOfKills = stringToInt(lines.front());
      lines.erase(lines.begin());
    }
  }

private:

  void setStrengthsFromFormula(ActorDefinition& d, const EntityStrength_t hpStrength) const;

  void finalizeDefinition(ActorDefinition& d) {
    actorDefinitions[d.devName] = d;
    d.description.resize(0);
  }

  Engine* eng;
};

#endif
