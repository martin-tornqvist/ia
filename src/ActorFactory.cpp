#include "ActorFactory.h"

#include "Engine.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Map.h"

Actor* ActorFactory::spawnActor(const ActorDevNames_t characterType, const coord& pos) {
  Monster* monster = NULL;

  switch(characterType) {
  case actor_zombie:
    monster = new ZombieClaw;
    break;
  case actor_zombieAxe:
    monster = new ZombieAxe;
    break;
  case actor_bloatedZombie:
    monster = new BloatedZombie;
    break;
  case actor_majorClaphamLee:
    monster = new MajorClaphamLee;
    break;
  case actor_lordOfPestilence:
    monster = new LordOfPestilence;
    break;
  case actor_lordOfShadows:
    monster = new LordOfShadows;
    break;
  case actor_lordOfSpiders:
    monster = new LordOfSpiders;
    break;
  case actor_lordOfSpirits:
    monster = new LordOfSpirits;
    break;
  case actor_deanHalsey:
    monster = new DeanHalsey;
    break;
  case actor_rat:
    monster = new Rat;
    break;
  case actor_ratThing:
    monster = new RatThing;
    break;
  case actor_brownJenkin:
    monster = new BrownJenkin;
    break;
  case actor_greenSpider:
    monster = new GreenSpider;
    break;
  case actor_blackSpider:
    monster = new BlackSpider;
    break;
  case actor_whiteSpider:
    monster = new WhiteSpider;
    break;
  case actor_lengSpider:
    monster = new LengSpider;
    break;
  case actor_fireHound:
    monster = new FireHound;
    break;
  case actor_ghost:
    monster = new Ghost;
    break;
  case actor_wraith:
    monster = new Wraith;
    break;
  case actor_phantasm:
    monster = new Phantasm;
    break;
  case actor_giantBat:
    monster = new GiantBat;
    break;
  case actor_cultistShotgun:
    monster = new CultistShotgun;
    break;
  case actor_cultistMachineGun:
    monster = new CultistMachineGun;
    break;
  case actor_cultistPistol:
    monster = new CultistPistol;
    break;
  case actor_cultistTeslaCannon:
    monster = new CultistTeslaCannon;
    break;
  case actor_cultistSpikeGun:
    monster = new CultistSpikeGun;
    break;
  case actor_cultistPriest:
    monster = new CultistPriest;
    break;
  case actor_keziahMason:
    monster = new KeziahMason;
    break;
  case actor_wolf:
    monster = new Wolf;
    break;
  case actor_miGo:
    monster = new MiGo;
    break;
  case actor_ghoul:
    monster = new Ghoul;
    break;
  case actor_shadow:
    monster = new Shadow;
    break;
  case actor_byakhee:
    monster = new Byakhee;
    break;
  case actor_mummy:
    monster = new Mummy;
    break;
  case actor_khephren:
    monster = new MummyUnique;
    break;
  case actor_nitokris:
    monster = new MummyUnique;
    break;
  case actor_deepOne:
    monster = new DeepOne;
    break;
  case actor_wormMass:
    monster = new WormMass;
    break;
  case actor_fireVampire:
    monster = new FireVampire;
    break;
  case actor_oozeGray:
    monster = new OozeGray;
    break;
  case actor_oozeClear:
    monster = new OozeClear;
    break;
  case actor_oozePutrid:
    monster = new OozePutrid;
    break;
  case actor_huntingHorror:
    monster = new HuntingHorror;
    break;

  default: {
  }
  break;
  }

  monster->place(pos, &(eng->actorData->actorDefinitions[characterType]), eng);

  if(monster->getArchetypeDefinition()->nrLeftAllowedToSpawn != -1) {
    monster->getArchetypeDefinition()->nrLeftAllowedToSpawn--;
  }

  eng->gameTime->insertActorInLoop(monster);

  return monster;
}

Actor* ActorFactory::spawnRandomActor(const coord& pos, const int SPAWN_LVL_OFFSET, const bool IS_AFTER_MAP_CREATION) {
  const int DLVL = eng->map->getDungeonLevel();
  vector<ActorDevNames_t> monsterCandidates;
  const unsigned int ACTORS_DEFINED = static_cast<unsigned int>(endOfActorDevNames);
  for(unsigned int i = 1; i < ACTORS_DEFINED; i++) {
    const ActorDefinition& def = eng->actorData->actorDefinitions[i];

    const bool IS_LVL_OK = DLVL + SPAWN_LVL_OFFSET >= def.spawnStandardMinLevel && DLVL <= def.spawnStandardMaxLevel;

    const bool IS_ALLOWED_TO_SPAWN =
      def.spawnRate != spawnNever &&
      def.nrLeftAllowedToSpawn != 0 &&
      (IS_AFTER_MAP_CREATION == false || def.isAllowedToSpawnAfterMapCreation);

    const bool IS_FEATURE_PASSABLE = eng->map->featuresStatic[pos.x][pos.y]->isMoveTypePassable(def.moveType);

    bool IS_NO_ACTOR_AT_POS = eng->mapTests->getActorAtPos(pos) == NULL;

    if(IS_LVL_OK && IS_ALLOWED_TO_SPAWN && IS_FEATURE_PASSABLE && IS_NO_ACTOR_AT_POS) {
      monsterCandidates.push_back(static_cast<ActorDevNames_t>(i));
    }
  }

  if(monsterCandidates.empty() == false) {
    const int ELEMENT = eng->dice.getInRange(0, monsterCandidates.size() - 1);
    const ActorDevNames_t monsterType = monsterCandidates.at(ELEMENT);
    return spawnActor(monsterType, pos);
  }

  return NULL;
}

Actor* ActorFactory::spawnRandomActorRelatedToSpecialRoom(const coord& pos, const SpecialRoom_t roomType, const int SPAWN_LVL_OFFSET) {
  const int DLVL = eng->map->getDungeonLevel();
  vector<ActorDevNames_t> monsterCandidates;
  const unsigned int ACTORS_DEFINED = static_cast<unsigned int>(endOfActorDevNames);
  for(unsigned int i = 1; i < ACTORS_DEFINED; i++) {
    const ActorDefinition& def = eng->actorData->actorDefinitions[i];

    const bool IS_LVL_OK = DLVL + SPAWN_LVL_OFFSET >= def.spawnStandardMinLevel && DLVL <= def.spawnStandardMaxLevel;

    const bool IS_ALLOWED_TO_SPAWN = def.spawnRate != spawnNever && def.nrLeftAllowedToSpawn != 0;

    const bool IS_FEATURE_PASSABLE = eng->map->featuresStatic[pos.x][pos.y]->isMoveTypePassable(def.moveType);

    bool IS_NO_ACTOR_AT_POS = eng->mapTests->getActorAtPos(pos) == NULL;

    bool isBelongToRoomType = false;
    for(unsigned int ii = 0; ii < def.nativeRooms.size(); ii++) {
      if(def.nativeRooms.at(ii) == roomType) {
        isBelongToRoomType = true;
      }
    }

    if(IS_LVL_OK && IS_ALLOWED_TO_SPAWN && IS_FEATURE_PASSABLE && IS_NO_ACTOR_AT_POS && isBelongToRoomType) {
      monsterCandidates.push_back(static_cast<ActorDevNames_t>(i));
    }
  }

  if(monsterCandidates.empty() == false) {
    const int ELEMENT = eng->dice.getInRange(0, monsterCandidates.size() - 1);
    const ActorDevNames_t monsterType = monsterCandidates.at(ELEMENT);
    return spawnActor(monsterType, pos);
  }

  return NULL;
}


void ActorFactory::deleteAllMonsters() {
  for(int i = 0; i < static_cast<int>(eng->gameTime->getLoopSize()); i++) {
    if(eng->gameTime->getActorAt(i) != eng->player) {
      eng->gameTime->eraseElement(i);
      i--;
    }
  }

  //Because all monsters was deleted, players target is NULL'd
  eng->player->target = NULL;
}

