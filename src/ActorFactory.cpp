#include "ActorFactory.h"

#include <assert.h>
#include <algorithm>

#include "Engine.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Renderer.h"
#include "MapParsing.h"

Actor* ActorFactory::makeActorFromId(const ActorId id) const {
  assert(id >= 1 && id < endOfActorIds);

  switch(id) {
    case actor_zombie:              return new ZombieClaw(eng);
    case actor_zombieAxe:           return new ZombieAxe(eng);
    case actor_bloatedZombie:       return new BloatedZombie(eng);
    case actor_majorClaphamLee:     return new MajorClaphamLee(eng);
    case actor_deanHalsey:          return new DeanHalsey(eng);
    case actor_rat:                 return new Rat(eng);
    case actor_ratThing:            return new RatThing(eng);
    case actor_brownJenkin:         return new BrownJenkin(eng);
    case actor_greenSpider:         return new GreenSpider(eng);
    case actor_redSpider:           return new RedSpider(eng);
    case actor_whiteSpider:         return new WhiteSpider(eng);
    case actor_shadowSpider:        return new ShadowSpider(eng);
    case actor_lengSpider:          return new LengSpider(eng);
    case actor_fireHound:           return new FireHound(eng);
    case actor_frostHound:          return new FrostHound(eng);
    case actor_zuul:                return new Zuul(eng);
    case actor_ghost:               return new Ghost(eng);
    case actor_wraith:              return new Wraith(eng);
    case actor_phantasm:            return new Phantasm(eng);
    case actor_giantBat:            return new GiantBat(eng);
    case actor_cultist:             return new Cultist(eng);
    case actor_cultistTeslaCannon:  return new CultistTeslaCannon(eng);
    case actor_cultistSpikeGun:     return new CultistSpikeGun(eng);
    case actor_cultistPriest:       return new CultistPriest(eng);
    case actor_keziahMason:         return new KeziahMason(eng);
    case actor_wolf:                return new Wolf(eng);
    case actor_flyingPolyp:         return new FlyingPolyp(eng);
    case actor_miGo:                return new MiGo(eng);
    case actor_ghoul:               return new Ghoul(eng);
    case actor_shadow:              return new Shadow(eng);
    case actor_byakhee:             return new Byakhee(eng);
    case actor_giantMantis:         return new GiantMantis(eng);
    case actor_giantLocust:         return new GiantLocust(eng);
    case actor_mummy:               return new Mummy(eng);
    case actor_khephren:            return new Khephren(eng);
    case actor_nitokris:            return new MummyUnique(eng);
    case actor_deepOne:             return new DeepOne(eng);
    case actor_wormMass:            return new WormMass(eng);
    case actor_dustVortex:          return new DustVortex(eng);
    case actor_fireVortex:          return new FireVortex(eng);
    case actor_frostVortex:         return new FrostVortex(eng);
    case actor_oozeBlack:           return new OozeBlack(eng);
    case actor_colourOutOfSpace:    return new ColourOutOfSpace(eng);
    case actor_oozeClear:           return new OozeClear(eng);
    case actor_oozePutrid:          return new OozePutrid(eng);
    case actor_oozePoison:          return new OozePoison(eng);
    case actor_chthonian:           return new Chthonian(eng);
    case actor_huntingHorror:       return new HuntingHorror(eng);

//  case actor_lordOfPestilence: return new LordOfPestilence;
//  case actor_lordOfShadows:    return new LordOfShadows;
//  case actor_lordOfSpiders:    return new LordOfSpiders;
//  case actor_lordOfSpirits:    return new LordOfSpirits;

    case actor_empty:
    case actor_player:
    case endOfActorIds: {} break;
  }
  return NULL;
}

Actor* ActorFactory::spawnActor(const ActorId id, const Pos& pos) const {
  Actor* const actor = makeActorFromId(id);

  actor->place(pos, eng.actorDataHandler->dataList[id]);

  if(actor->getData().nrLeftAllowedToSpawn != -1) {
    actor->getData().nrLeftAllowedToSpawn--;
  }

  eng.gameTime->insertActorInLoop(actor);

  return actor;
}

void ActorFactory::deleteAllMonsters() const {
  vector<Actor*>& actors = eng.gameTime->actors_;

  for(size_t i = 0; i < actors.size(); i++) {
    if(actors.at(i) != eng.player) {
      eng.gameTime->eraseActorInElement(i);
      i--;
    }
  }

  eng.player->target = NULL;
}

void ActorFactory::summonMonsters(
  const Pos& origin, const vector<ActorId>& monsterIds,
  const bool MAKE_MONSTERS_AWARE,
  Actor* const actorToSetAsLeader,
  vector<Monster*>* monstersRet) const {

  if(monstersRet != NULL) {
    monstersRet->resize(0);
  }

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true, eng), blockers);
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
    const ActorId id  = monsterIds.at(i);

    Actor*   const actor    = spawnActor(id, pos);
    Monster* const monster  = dynamic_cast<Monster*>(actor);

    if(monstersRet != NULL) {
      monstersRet->push_back(monster);
    }
    if(actorToSetAsLeader != NULL) {
      monster->leader = actorToSetAsLeader;
    }
    if(MAKE_MONSTERS_AWARE) {
      monster->awareOfPlayerCounter_ = monster->getData().nrTurnsAwarePlayer;
    }

    if(eng.player->isSeeingActor(*actor, NULL)) {
      positionsToAnimate.push_back(pos);
    }
  }

  eng.renderer->drawBlastAnimAtPositions(positionsToAnimate, clrMagenta);
}


