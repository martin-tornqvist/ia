#include "ActorFactory.h"

#include <algorithm>
#include <assert.h>

#include "ActorMon.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Render.h"
#include "MapParsing.h"
#include "Utils.h"
#include "Actor.h"

using namespace std;

namespace ActorFactory
{

namespace
{

Actor* mkActorFromId(const ActorId id)
{
  switch (id)
  {
    case ActorId::player:               return new Player();
    case ActorId::zombie:               return new ZombieClaw();
    case ActorId::zombieAxe:            return new ZombieAxe();
    case ActorId::bloatedZombie:        return new BloatedZombie();
    case ActorId::majorClaphamLee:      return new MajorClaphamLee();
    case ActorId::deanHalsey:           return new DeanHalsey();
    case ActorId::rat:                  return new Rat();
    case ActorId::ratThing:             return new RatThing();
    case ActorId::brownJenkin:          return new BrownJenkin();
    case ActorId::greenSpider:          return new GreenSpider();
    case ActorId::redSpider:            return new RedSpider();
    case ActorId::whiteSpider:          return new WhiteSpider();
    case ActorId::shadowSpider:         return new ShadowSpider();
    case ActorId::lengSpider:           return new LengSpider();
    case ActorId::fireHound:            return new FireHound();
    case ActorId::frostHound:           return new FrostHound();
    case ActorId::zuul:                 return new Zuul();
    case ActorId::ghost:                return new Ghost();
    case ActorId::wraith:               return new Wraith();
    case ActorId::phantasm:             return new Phantasm();
    case ActorId::giantBat:             return new GiantBat();
    case ActorId::cultist:              return new Cultist();
    case ActorId::cultistElectric:      return new CultistElectric();
    case ActorId::cultistSpikeGun:      return new CultistSpikeGun();
    case ActorId::cultistPriest:        return new CultistPriest();
    case ActorId::keziahMason:          return new KeziahMason();
    case ActorId::lengElder:            return new LengElder();
    case ActorId::wolf:                 return new Wolf();
    case ActorId::flyingPolyp:          return new FlyingPolyp();
    case ActorId::miGo:                 return new MiGo();
    case ActorId::ghoul:                return new Ghoul();
    case ActorId::shadow:               return new Shadow();
    case ActorId::byakhee:              return new Byakhee();
    case ActorId::giantMantis:          return new GiantMantis();
    case ActorId::locust:               return new GiantLocust();
    case ActorId::mummy:                return new Mummy();
    case ActorId::khephren:             return new Khephren();
    case ActorId::nitokris:             return new MummyUnique();
    case ActorId::deepOne:              return new DeepOne();
    case ActorId::wormMass:             return new WormMass();
    case ActorId::dustVortex:           return new DustVortex();
    case ActorId::fireVortex:           return new FireVortex();
    case ActorId::frostVortex:          return new FrostVortex();
    case ActorId::oozeBlack:            return new OozeBlack();
    case ActorId::colourOOSpace:        return new ColorOOSpace();
    case ActorId::oozeClear:            return new OozeClear();
    case ActorId::oozePutrid:           return new OozePutrid();
    case ActorId::oozePoison:           return new OozePoison();
    case ActorId::chthonian:            return new Chthonian();
    case ActorId::huntingHorror:        return new HuntingHorror();
    case ActorId::sentryDrone:          return new SentryDrone();
    case ActorId::mold:                 return new Mold();
    case ActorId::gasSpore:             return new GasSpore();
    case ActorId::theDarkOne:           return new TheDarkOne();

    case ActorId::END: {} break;
  }
  return nullptr;
}

} //namespace

Actor* mk(const ActorId id, const Pos& pos)
{
  Actor* const actor = mkActorFromId(id);

  actor->place(pos, ActorData::data[int(id)]);

  if (actor->getData().nrLeftAllowedToSpawn != -1)
  {
    actor->getData().nrLeftAllowedToSpawn--;
  }

  GameTime::addActor(actor);

  return actor;
}

void deleteAllMon()
{
  vector<Actor*>& actors = GameTime::actors_;

  for (size_t i = 0; i < actors.size(); ++i)
  {
    if (actors[i] != Map::player)
    {
      GameTime::eraseActorInElement(i);
      i--;
    }
  }
}

void summonMon(const Pos& origin, const vector<ActorId>& monsterIds,
               const bool MAKE_MONSTERS_AWARE, Actor* const actorToSetAsLeader,
               vector<Mon*>* monstersRet)
{
  if (monstersRet) {monstersRet->clear();}

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(true), blocked);
  vector<Pos> freeCells;
  Utils::mkVectorFromBoolMap(false, blocked, freeCells);
  sort(begin(freeCells), end(freeCells), IsCloserToPos(origin));

  const int NR_FREE_CELLS   = freeCells.size();
  const int NR_MONSTER_IDS  = monsterIds.size();
  const int NR_TO_SPAWN     = min(NR_FREE_CELLS, NR_MONSTER_IDS);

  vector<Pos> positionsToAnimate;

  for (int i = 0; i < NR_TO_SPAWN; ++i)
  {
    const Pos&    pos   = freeCells[i];
    const ActorId id    = monsterIds[i];
    Actor* const  actor = mk(id, pos);
    Mon* const    mon   = static_cast<Mon*>(actor);

    if (monstersRet)
    {
      monstersRet->push_back(mon);
    }
    if (actorToSetAsLeader)
    {
      mon->leader_ = actorToSetAsLeader;
    }
    if (MAKE_MONSTERS_AWARE)
    {
      mon->awareCounter_ = mon->getData().nrTurnsAware;
    }

    if (Map::player->isSeeingActor(*actor, nullptr))
    {
      positionsToAnimate.push_back(pos);
    }
  }

  Render::drawBlastAtCells(positionsToAnimate, clrMagenta);
}

} //ActorFactory
