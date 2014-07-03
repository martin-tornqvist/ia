#include "ActorFactory.h"

#include <algorithm>
#include <assert.h>

#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "Utils.h"
#include "Actor.h"

using namespace std;

namespace ActorFactory {

namespace {

Actor* mkActorFromId(const ActorId id) {
  assert(id >= 1 && id < endOfActorIds);

  switch(id) {
    case actor_player:              return new Player();
    case actor_zombie:              return new ZombieClaw();
    case actor_zombieAxe:           return new ZombieAxe();
    case actor_bloatedZombie:       return new BloatedZombie();
    case actor_majorClaphamLee:     return new MajorClaphamLee();
    case actor_deanHalsey:          return new DeanHalsey();
    case actor_rat:                 return new Rat();
    case actor_ratThing:            return new RatThing();
    case actor_brownJenkin:         return new BrownJenkin();
    case actor_greenSpider:         return new GreenSpider();
    case actor_redSpider:           return new RedSpider();
    case actor_whiteSpider:         return new WhiteSpider();
    case actor_shadowSpider:        return new ShadowSpider();
    case actor_lengSpider:          return new LengSpider();
    case actor_fireHound:           return new FireHound();
    case actor_frostHound:          return new FrostHound();
    case actor_zuul:                return new Zuul();
    case actor_ghost:               return new Ghost();
    case actor_wraith:              return new Wraith();
    case actor_phantasm:            return new Phantasm();
    case actor_giantBat:            return new GiantBat();
    case actor_cultist:             return new Cultist();
    case actor_cultistTeslaCannon:  return new CultistTeslaCannon();
    case actor_cultistSpikeGun:     return new CultistSpikeGun();
    case actor_cultistPriest:       return new CultistPriest();
    case actor_keziahMason:         return new KeziahMason();
    case actor_wolf:                return new Wolf();
    case actor_flyingPolyp:         return new FlyingPolyp();
    case actor_miGo:                return new MiGo();
    case actor_ghoul:               return new Ghoul();
    case actor_shadow:              return new Shadow();
    case actor_byakhee:             return new Byakhee();
    case actor_giantMantis:         return new GiantMantis();
    case actor_locust:              return new GiantLocust();
    case actor_mummy:               return new Mummy();
    case actor_khephren:            return new Khephren();
    case actor_nitokris:            return new MummyUnique();
    case actor_deepOne:             return new DeepOne();
    case actor_wormMass:            return new WormMass();
    case actor_dustVortex:          return new DustVortex();
    case actor_fireVortex:          return new FireVortex();
    case actor_frostVortex:         return new FrostVortex();
    case actor_oozeBlack:           return new OozeBlack();
    case actor_colourOOSpace:       return new ColourOOSpace();
    case actor_oozeClear:           return new OozeClear();
    case actor_oozePutrid:          return new OozePutrid();
    case actor_oozePoison:          return new OozePoison();
    case actor_chthonian:           return new Chthonian();
    case actor_huntingHorror:       return new HuntingHorror();

//  case actor_lordOfPestilence: return new LordOfPestilence;
//  case actor_lordOfShadows:    return new LordOfShadows;
//  case actor_lordOfSpiders:    return new LordOfSpiders;
//  case actor_lordOfSpirits:    return new LordOfSpirits;

    case actor_empty:
    case endOfActorIds: {} break;
  }
  return nullptr;
}

} //namespace

Actor* mk(const ActorId id, const Pos& pos) {
  Actor* const actor = mkActorFromId(id);

  actor->place(pos, ActorData::data[id]);

  if(actor->getData().nrLeftAllowedToSpawn != -1) {
    actor->getData().nrLeftAllowedToSpawn--;
  }

  GameTime::insertActorInLoop(actor);

  return actor;
}

void deleteAllMonsters() {
  vector<Actor*>& actors = GameTime::actors_;

  for(size_t i = 0; i < actors.size(); ++i) {
    if(actors.at(i) != Map::player) {
      GameTime::eraseActorInElement(i);
      i--;
    }
  }
}

void summonMonsters(const Pos& origin, const vector<ActorId>& monsterIds,
                    const bool MAKE_MONSTERS_AWARE,
                    Actor* const actorToSetAsLeader,
                    vector<Monster*>* monstersRet) {

  if(monstersRet) {monstersRet->resize(0);}

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);
  vector<Pos> freeCells;
  Utils::mkVectorFromBoolMap(false, blocked, freeCells);
  sort(freeCells.begin(), freeCells.end(), IsCloserToPos(origin));

  const int NR_FREE_CELLS   = freeCells.size();
  const int NR_MONSTER_IDS  = monsterIds.size();
  const int NR_TO_SPAWN     = min(NR_FREE_CELLS, NR_MONSTER_IDS);

  vector<Pos> positionsToAnimate;
  positionsToAnimate.resize(0);

  for(int i = 0; i < NR_TO_SPAWN; ++i) {
    const Pos&    pos = freeCells.at(i);
    const ActorId id  = monsterIds.at(i);

    Actor*   const actor    = mk(id, pos);
    Monster* const monster  = static_cast<Monster*>(actor);

    if(monstersRet) {
      monstersRet->push_back(monster);
    }
    if(actorToSetAsLeader) {
      monster->leader = actorToSetAsLeader;
    }
    if(MAKE_MONSTERS_AWARE) {
      monster->awareOfPlayerCounter_ = monster->getData().nrTurnsAwarePlayer;
    }

    if(Map::player->isSeeingActor(*actor, nullptr)) {
      positionsToAnimate.push_back(pos);
    }
  }

  Renderer::drawBlastAnimAtPositions(positionsToAnimate, clrMagenta);
}

} //ActorFactory
