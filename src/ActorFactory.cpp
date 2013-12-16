#include "ActorFactory.h"

#include <assert.h>
#include <algorithm>

#include "Engine.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Renderer.h"
#include "MapParsing.h"

Actor* ActorFactory::makeActorFromId(const ActorId_t id) const {
  assert(id >= 1 && id < endOfActorIds);

  switch(id) {
    case actor_zombie:              return new ZombieClaw(eng); break;
    case actor_zombieAxe:           return new ZombieAxe(eng); break;
    case actor_bloatedZombie:       return new BloatedZombie(eng); break;
    case actor_majorClaphamLee:     return new MajorClaphamLee(eng); break;
    case actor_deanHalsey:          return new DeanHalsey(eng); break;
    case actor_rat:                 return new Rat(eng); break;
    case actor_ratThing:            return new RatThing(eng); break;
    case actor_brownJenkin:         return new BrownJenkin(eng); break;
    case actor_greenSpider:         return new GreenSpider(eng); break;
    case actor_redSpider:           return new RedSpider(eng); break;
    case actor_whiteSpider:         return new WhiteSpider(eng); break;
    case actor_shadowSpider:        return new ShadowSpider(eng); break;
    case actor_lengSpider:          return new LengSpider(eng); break;
    case actor_fireHound:           return new FireHound(eng); break;
    case actor_frostHound:          return new FrostHound(eng); break;
    case actor_ghost:               return new Ghost(eng); break;
    case actor_wraith:              return new Wraith(eng); break;
    case actor_phantasm:            return new Phantasm(eng); break;
    case actor_giantBat:            return new GiantBat(eng); break;
    case actor_cultist:             return new Cultist(eng); break;
    case actor_cultistTeslaCannon:  return new CultistTeslaCannon(eng); break;
    case actor_cultistSpikeGun:     return new CultistSpikeGun(eng); break;
    case actor_cultistPriest:       return new CultistPriest(eng); break;
    case actor_keziahMason:         return new KeziahMason(eng); break;
    case actor_wolf:                return new Wolf(eng); break;
    case actor_miGo:                return new MiGo(eng); break;
    case actor_ghoul:               return new Ghoul(eng); break;
    case actor_shadow:              return new Shadow(eng); break;
    case actor_byakhee:             return new Byakhee(eng); break;
    case actor_giantMantis:         return new GiantMantis(eng); break;
    case actor_giantLocust:         return new GiantLocust(eng); break;
    case actor_mummy:               return new Mummy(eng); break;
    case actor_khephren:            return new Khephren(eng); break;
    case actor_nitokris:            return new MummyUnique(eng); break;
    case actor_deepOne:             return new DeepOne(eng); break;
    case actor_wormMass:            return new WormMass(eng); break;
    case actor_dustVortex:          return new DustVortex(eng); break;
    case actor_fireVortex:          return new FireVortex(eng); break;
    case actor_frostVortex:         return new FrostVortex(eng); break;
    case actor_oozeBlack:           return new OozeBlack(eng); break;
    case actor_colourOutOfSpace:    return new ColourOutOfSpace(eng); break;
    case actor_oozeClear:           return new OozeClear(eng); break;
    case actor_oozePutrid:          return new OozePutrid(eng); break;
    case actor_oozePoison:          return new OozePoison(eng); break;
    case actor_huntingHorror:       return new HuntingHorror(eng); break;

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

  actor->place(pos, eng.actorDataHandler->dataList[id]);

  if(actor->getData()->nrLeftAllowedToSpawn != -1) {
    actor->getData()->nrLeftAllowedToSpawn--;
  }

  eng.gameTime->insertActorInLoop(actor);

  return actor;
}

void ActorFactory::deleteAllMonsters() const {
  for(int i = 0; i < eng.gameTime->getNrActors(); i++) {
    if(&(eng.gameTime->getActorAtElement(i)) != eng.player) {
      eng.gameTime->eraseActorInElement(i);
      i--;
    }
  }

  eng.player->target = NULL;
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
  MapParser::parse(
    CellPredBlocksBodyType(bodyType_normal, true, eng), blockers);
  vector<Pos> freeCells;
  eng.basicUtils->makeVectorFromBoolMap(false, blockers, freeCells);
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

    if(eng.player->checkIfSeeActor(*actor, NULL)) {
      positionsToAnimate.push_back(pos);
    }
  }

  eng.renderer->drawBlastAnimationAtPositions(positionsToAnimate, clrMagenta);
}


