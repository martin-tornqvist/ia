#include "ActorFactory.h"

#include <algorithm>

#include "Engine.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Renderer.h"

Actor* ActorFactory::makeActorFromId(const ActorId_t id) const {
  if(id < 1 || id >= endOfActorIds) {
    throw runtime_error("Bad actor id");
  }

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
    case actor_frostHound:          return new FrostHound;            break;
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
    case actor_frostVortex:         return new FrostVortex;           break;
    case actor_oozeBlack:           return new OozeBlack;             break;
    case actor_colourOutOfSpace:    return new ColourOutOfSpace;      break;
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

    case actor_empty:
    case actor_player:
    case endOfActorIds: {} break;
  }
  return NULL;
}

Actor* ActorFactory::spawnActor(const ActorId_t id, const Pos& pos) const {
  Actor* const actor = makeActorFromId(id);

  actor->place(pos, &(eng->actorDataHandler->dataList[id]), eng);

  if(actor->getData()->nrLeftAllowedToSpawn != -1) {
    actor->getData()->nrLeftAllowedToSpawn--;
  }

  eng->gameTime->insertActorInLoop(actor);

  return actor;
}

void ActorFactory::deleteAllMonsters() const {
  for(int i = 0; i < int(eng->gameTime->getLoopSize()); i++) {
    if(eng->gameTime->getActorAt(i) != eng->player) {
      eng->gameTime->eraseElement(i);
      i--;
    }
  }

  eng->player->target = NULL;
}

void ActorFactory::summonMonsters(
  const Pos& origin, const vector<ActorId_t>& monsterIds,
  const bool MAKE_MONSTERS_AWARE,
  Actor* const actorToSetAsLeader,
  vector<Monster*>* monstersRet) const {

  if(monstersRet != NULL) {
    monstersRet->resize(0);
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyType(
    actorBodyType_normal, blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<Pos> freeCells;
  eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
  sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(origin, eng));

  const int NR_FREE_CELLS   = freeCells.size();
  const int NR_MONSTER_IDS  = monsterIds.size();
  const int NR_TO_SPAWN     = min(NR_FREE_CELLS, NR_MONSTER_IDS);

  vector<Pos> positionsToAnimate;
  positionsToAnimate.resize(0);

  for(int i = 0; i < NR_TO_SPAWN; i++) {
    const Pos&      pos = freeCells.at(i);
    const ActorId_t id  = monsterIds.at(i);

    Actor*   const actor    = spawnActor(id, pos);
    Monster* const monster  = dynamic_cast<Monster*>(actor);

    if(monstersRet != NULL) {
      monstersRet->push_back(monster);
    }
    if(actorToSetAsLeader != NULL) {
      monster->leader = actorToSetAsLeader;
    }
    if(MAKE_MONSTERS_AWARE) {
      monster->playerAwarenessCounter = monster->getData()->nrTurnsAwarePlayer;
    }

    if(eng->player->checkIfSeeActor(*actor, NULL)) {
      positionsToAnimate.push_back(pos);
    }
  }

  eng->renderer->drawBlastAnimationAtPositions(positionsToAnimate, clrMagenta);
}


