#include "ActorFactory.h"

#include "Engine.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Map.h"


Actor* ActorFactory::makeActorFromId(const ActorId_t id) {
  switch(id) {
    case actor_zombie:              return new ZombieClaw;            break;
    case actor_zombieAxe:           return new ZombieAxe;             break;
    case actor_bloatedZombie:       return new BloatedZombie;         break;
    case actor_majorClaphamLee:     return new MajorClaphamLee;       break;
    case actor_deanHalsey:          return new DeanHalsey;            break;
    case actor_rat:                 return new Rat;                   break;
    case actor_ratThing:            return new RatThing;              break;
    case actor_brownJenkin:         return new BrownJenkin;           break;
    case actor_greenSpider:         return new GreenSpider;           break;
    case actor_redSpider:           return new RedSpider;             break;
    case actor_whiteSpider:         return new WhiteSpider;           break;
    case actor_shadowSpider:        return new ShadowSpider;          break;
    case actor_lengSpider:          return new LengSpider;            break;
    case actor_fireHound:           return new FireHound;             break;
    case actor_ghost:               return new Ghost;                 break;
    case actor_wraith:              return new Wraith;                break;
    case actor_phantasm:            return new Phantasm;              break;
    case actor_giantBat:            return new GiantBat;              break;
    case actor_cultist:             return new Cultist;               break;
    case actor_cultistTeslaCannon:  return new CultistTeslaCannon;    break;
    case actor_cultistSpikeGun:     return new CultistSpikeGun;       break;
    case actor_cultistPriest:       return new CultistPriest;         break;
    case actor_keziahMason:         return new KeziahMason;           break;
    case actor_wolf:                return new Wolf;                  break;
    case actor_miGo:                return new MiGo;                  break;
    case actor_ghoul:               return new Ghoul;                 break;
    case actor_shadow:              return new Shadow;                break;
    case actor_byakhee:             return new Byakhee;               break;
    case actor_giantMantis:         return new GiantMantis;           break;
    case actor_giantLocust:         return new GiantLocust;           break;
    case actor_mummy:               return new Mummy;                 break;
    case actor_khephren:            return new Khephren;              break;
    case actor_nitokris:            return new MummyUnique;           break;
    case actor_deepOne:             return new DeepOne;               break;
    case actor_wormMass:            return new WormMass;              break;
    case actor_dustVortex:          return new DustVortex;            break;
    case actor_fireVortex:          return new FireVortex;            break;
    case actor_oozeBlack:           return new OozeBlack;             break;
    case actor_oozeClear:           return new OozeClear;             break;
    case actor_oozePutrid:          return new OozePutrid;            break;
    case actor_oozePoison:          return new OozePoison;            break;
    case actor_huntingHorror:       return new HuntingHorror;         break;

//  case actor_lordOfPestilence:
//    return new LordOfPestilence;
//    break;
//  case actor_lordOfShadows:
//    return new LordOfShadows;
//    break;
//  case actor_lordOfSpiders:
//    return new LordOfSpiders;
//    break;
//  case actor_lordOfSpirits:
//    return new LordOfSpirits;
//    break;

    default: {
      tracer << "[WARNING] Bad actor ID given in ActorFactory::makeActorFromId()" << endl;
      return NULL;
    }
    break;
  }
}

Actor* ActorFactory::spawnActor(const ActorId_t id, const coord& pos) {
  Actor* const actor = makeActorFromId(id);

  actor->place(pos, &(eng->actorData->actorDefinitions[id]), eng);

  if(actor->getDef()->nrLeftAllowedToSpawn != -1) {
    actor->getDef()->nrLeftAllowedToSpawn--;
  }

  eng->gameTime->insertActorInLoop(actor);

  return actor;
}

Actor* ActorFactory::spawnRandomActor(const coord& pos, const int SPAWN_LVL_OFFSET) {
  const int DLVL = eng->map->getDungeonLevel();
  vector<ActorId_t> monsterCandidates;
  const unsigned int ACTORS_DEFINED = static_cast<unsigned int>(endOfActorIds);
  for(unsigned int i = 1; i < ACTORS_DEFINED; i++) {
    const ActorDefinition& def = eng->actorData->actorDefinitions[i];

    const bool IS_LVL_OK = DLVL + SPAWN_LVL_OFFSET >= def.spawnMinLevel && DLVL <= def.spawnMaxLevel;

    const bool IS_ALLOWED_TO_SPAWN = def.isAutoSpawnAllowed && def.nrLeftAllowedToSpawn != 0;

    const bool IS_FEATURE_PASSABLE = eng->map->featuresStatic[pos.x][pos.y]->isMoveTypePassable(def.moveType);

    bool IS_NO_ACTOR_AT_POS = eng->mapTests->getActorAtPos(pos) == NULL;

    if(IS_LVL_OK && IS_ALLOWED_TO_SPAWN && IS_FEATURE_PASSABLE && IS_NO_ACTOR_AT_POS) {
      monsterCandidates.push_back(static_cast<ActorId_t>(i));
    }
  }

  if(monsterCandidates.empty() == false) {
    const int ELEMENT = eng->dice.getInRange(0, monsterCandidates.size() - 1);
    const ActorId_t monsterType = monsterCandidates.at(ELEMENT);
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

  eng->player->target = NULL;
}

