#include "Init.h"

#include "Bot.h"

#include <assert.h>
#include <algorithm>
#include <vector>

#include "Properties.h"
#include "Actor.h"
#include "Feature.h"
#include "Input.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "Attack.h"
#include "FeatureDoor.h"
#include "Inventory.h"
#include "ActorMon.h"
#include "MapParsing.h"
#include "Utils.h"
#include "GameTime.h"
#include "MapTravel.h"

using namespace std;

namespace Bot
{

namespace
{

std::vector<Pos> curPath_;

void findPathToStairs()
{
  curPath_.clear();

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  Pos stairPos(-1, -1);

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      const auto curId = Map::cells[x][y].rigid->getId();
      if (curId == FeatureId::stairs)
      {
        blocked[x][y] = false;
        stairPos.set(x, y);
      }
      else if (curId == FeatureId::door)
      {
        blocked[x][y] = false;
      }
    }
  }
  assert(stairPos != Pos(-1, -1));

  PathFind::run(Map::player->pos, stairPos, blocked, curPath_);

  assert(!curPath_.empty());
  assert(curPath_.front() == stairPos);
}

bool walkToAdjCell(const Pos& p)
{
  assert(Utils::isPosAdj(Map::player->pos, p, true));

  char key = '0' + int(DirUtils::getDir(p - Map::player->pos));

  //Occasionally randomize movement
  if (Rnd::oneIn(5)) {key = '0' + Rnd::range(1, 9);}

  Input::handleMapModeKeyPress(KeyData(key));

  return Map::player->pos == p;
}

} //namespace

void init()
{
  curPath_.clear();
}

void act()
{
  //=======================================================================
  // TESTS
  //=======================================================================
  for (Actor* actor : GameTime::actors_)
  {
#ifdef NDEBUG
    (void)actor;
#else
    assert(Utils::isPosInsideMap(actor->pos));
#endif
  }

  //=======================================================================

  //Check if we are finished with the current run, if so, go back to DLVL 1
  if (Map::dlvl >= LAST_CAVERN_LVL)
  {
    TRACE << "Starting new run on first dungeon level" << endl;
    Map::dlvl = 1;
    MapTravel::init();
    return;
  }

  PropHandler& propHandler = Map::player->getPropHandler();

  //Occasionally apply RFear (to avoid getting stuck on fear-causing monsters)
  if (Rnd::oneIn(7))
  {
    propHandler.tryApplyProp(new PropRFear(PropTurns::specific, 4), true);
  }

  //Occasionally apply Burning to a random actor (helps to avoid getting stuck)
  if (Rnd::oneIn(10))
  {
    const int ELEMENT = Rnd::range(0, GameTime::actors_.size() - 1);
    Actor* const actor = GameTime::actors_[ELEMENT];
    if (actor != Map::player)
    {
      actor->getPropHandler().tryApplyProp(new PropBurning(PropTurns::std), true);
    }
  }

  //Occasionally teleport (to avoid getting stuck)
  if (Rnd::oneIn(200))
  {
    Map::player->teleport(false);
  }

  //Occasionally send a TAB command to attack nearby monsters
  if (Rnd::coinToss())
  {
    Input::handleMapModeKeyPress(KeyData(SDLK_TAB));
    return;
  }

  //Occasionally apply a random property to exercise the prop code
  if (Rnd::oneIn(10))
  {
    vector<PropId> propBucket;

    for (int i = 0; i < endOfPropIds; ++i)
    {
      if (PropData::data[i].allowTestOnBot) {propBucket.push_back(PropId(i));}
    }

    PropId      propId  = propBucket[Rnd::range(0, propBucket.size() - 1)];
    Prop* const prop    = propHandler.mkProp(propId, PropTurns::specific, 5);

    propHandler.tryApplyProp(prop, true);
  }

  //Handle blocking door
  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      const Pos p(Map::player->pos + Pos(dx, dy));
      auto* const f = Map::cells[p.x][p.y].rigid;
      if (f->getId() == FeatureId::door)
      {
        Door* const door = static_cast<Door*>(f);
        door->reveal(false);
        if (door->isStuck())
        {
          f->hit(DmgType::physical, DmgMethod::kick, Map::player);
          return;
        }
      }
    }
  }

  //If we are terrified, wait in place
  bool props[endOfPropIds];
  Map::player->getPropHandler().getActivePropIds(props);

  if (props[propTerrified])
  {
    if (walkToAdjCell(Map::player->pos))
    {
      return;
    }
  }

  findPathToStairs();

  walkToAdjCell(curPath_.back());
}

} //Bot
