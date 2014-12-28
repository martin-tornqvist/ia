#include "Ai.h"

#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Utils.h"
#include "FeatureMob.h"
#include "FeatureDoor.h"
#include "ActorMon.h"
#include "LineCalc.h"
#include "MapParsing.h"
#include "GameTime.h"
#include "Fov.h"

using namespace std;

namespace Ai
{

namespace Action
{

bool tryCastRandomSpell(Mon& mon)
{
  if (mon.isAlive() && mon.spellCoolDownCur_ == 0)
  {
    if (!mon.getPropHandler().allowRead(false))
    {
      return false;
    }

    if (!mon.spellsKnown_.empty())
    {
      vector<Spell*> spellBucket = mon.spellsKnown_;

      std::random_shuffle(begin(spellBucket), end(spellBucket));

      while (!spellBucket.empty())
      {
        Spell* const spell = spellBucket.back();

        if (spell->allowMonCastNow(mon))
        {
          const int CUR_SPI = mon.getSpi();
          const int SPELL_MAX_SPI = spell->getSpiCost(false, &mon).upper;

          //Cast spell if max spirit cost is lower than current spirit,
          if (SPELL_MAX_SPI < CUR_SPI)
          {
            spell->cast(&mon, true);
            return true;
          }

          //This point reached means SPI was lower than the spells potential cost
          const int CUR_HP  = mon.getHp();
          const int MAX_HP  = mon.getHpMax(true);

          //If monster is not allied to player, with a small chance, cast the spell
          //anyway if HP is low.
          if (!Map::player->isLeaderOf(&mon) && CUR_HP < (MAX_HP / 3) && Rnd::oneIn(20))
          {
            if (Map::player->isSeeingActor(mon, nullptr))
            {
              Log::addMsg(mon.getNameThe() + " looks desperate.");
            }
            spell->cast(&mon, true);
            return true;
          }
          return false;
        }
        else //Spell does not allow monster to cast now
        {
          spellBucket.pop_back();
        }
      }
    }
  }
  return false;
}

bool handleClosedBlockingDoor(Mon& mon, vector<Pos> path)
{
  if (mon.isAlive() && !path.empty())
  {
    const Pos& p = path.back();
    Feature* const f = Map::cells[p.x][p.y].rigid;
    if (f->getId() == FeatureId::door)
    {
      Door* const door = static_cast<Door*>(f);
      bool props[endOfPropIds];
      mon.getPropHandler().getPropIds(props);
      if (!door->canMove(props))
      {
        if (!door->isStuck())
        {
          if (mon.getData().canOpenDoors)
          {
            door->tryOpen(&mon);
            return true;
          }
          else if (mon.getData().canBashDoors)
          {
            door->hit(DmgType::physical, DmgMethod::kick, &mon);
            return true;
          }
        }
        else if (mon.getData().canBashDoors)
        {
          door->hit(DmgType::physical, DmgMethod::kick, &mon);
          return true;
        }
      }
    }
  }
  return false;
}

bool handleInventory(Mon& mon)
{
  (void)mon;
  return false;
}

//Helper functions for makeRoomForFriend()
namespace
{

//Check if acting monster is on a line between player and other monster
bool checkIfBlockingMon(const Pos& pos, Mon& other)
{
  vector<Pos> line;
  LineCalc::calcNewLine(other.pos, Map::player->pos, true, 9999, false, line);

  for (const Pos& posInLine : line) {if (posInLine == pos) {return true;}}
  return false;
}

//Returns all free positions around the acting monster that is further
//from the player than the monster's current position
void getMoveBucket(Mon& self, vector<Pos>& dirsToMk)
{

  dirsToMk.clear();

  const int PLAYER_X = Map::player->pos.x;
  const int PLAYER_Y = Map::player->pos.y;
  const int OLD_X = self.pos.x;
  const int OLD_Y = self.pos.y;

  bool blocked[MAP_W][MAP_H];
  MapParse::run(CellCheck::BlocksActor(self, true), blocked);

  for (int x = -1; x <= 1; ++x)
  {
    for (int y = -1; y <= 1; ++y)
    {
      if (x != OLD_X || y != OLD_Y)
      {
        const int NEW_X = OLD_X + x;
        const int NEW_Y = OLD_Y + y;
        const int DIST_CUR = Utils::kingDist(OLD_X, OLD_Y, PLAYER_X, PLAYER_Y);
        const int DIST_NEW = Utils::kingDist(NEW_X, NEW_Y, PLAYER_X, PLAYER_Y);

        if (DIST_NEW <= DIST_CUR)
        {
          if (!blocked[NEW_X][NEW_Y]) {dirsToMk.push_back(Pos(NEW_X, NEW_Y));}
        }
      }
    }
  }
}

/*
 The purpose of the following test is to handle this situation:
 #####
 #.A.#
 #@#B#
 #####
 */
bool isAdjAndNoVision(const Mon& self, Mon& other,
                      bool blockedLos[MAP_W][MAP_H])
{
  //If the pal is next to me
  if (Utils::isPosAdj(self.pos, other.pos, false))
  {
    //If pal does not see player
    if (!other.isSeeingActor(*Map::player, blockedLos)) {return true;}
  }
  return false;
}

} //namespace

bool makeRoomForFriend(Mon& mon)
{
  if (mon.isAlive())
  {

    bool blockedLos[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksLos(), blockedLos);

    if (mon.isSeeingActor(*Map::player, blockedLos))
    {

      //Loop through all actors
      for (Actor* actor : GameTime::actors_)
      {
        if (actor != Map::player && actor != &mon &&
            actor->isAlive())
        {

          Mon* other = static_cast<Mon*>(actor);

          bool isOtherAdjWithNoLos = isAdjAndNoVision(mon, *other, blockedLos);

          //Other monster can see player, or it's an adjacent monster that does not see
          //the player?
          if (other->isSeeingActor(*Map::player, blockedLos) || isOtherAdjWithNoLos)
          {

            //If we are blocking a pal, check every neighbouring position that is at
            //equal or closer distance to the player, to check whether they are fine.

            //TODO: Vision must be checked from the cell candidates!

            if (checkIfBlockingMon(mon.pos, *other) || isOtherAdjWithNoLos)
            {
              // Get a list of neighbouring free cells
              vector<Pos> posBucket;
              getMoveBucket(mon, posBucket);

              //Sort the list by closeness to player
              IsCloserToPos cmp(Map::player->pos);
              sort(posBucket.begin(), posBucket.end(), cmp);

              //Test positions until one is found that is not blocking another monster
              for (const auto& targetPos : posBucket)
              {
                bool isGoodCandidateFound = true;

                for (Actor* actor2 : GameTime::actors_)
                {
                  if (actor2 != Map::player && actor2 != &mon)
                  {
                    other = static_cast<Mon*>(actor2);
                    if (other->isSeeingActor(*Map::player, blockedLos))
                    {
                      if (checkIfBlockingMon(targetPos, *other))
                      {
                        isGoodCandidateFound = false;
                        break;
                      }
                    }
                  }
                }

                if (isGoodCandidateFound)
                {
                  const Pos offset = targetPos - mon.pos;
                  mon.moveDir(DirUtils::getDir(offset));
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}

//Helper function for moveToRandomAdjCell()
namespace
{

Dir getDirToRndAdjFreeCell(Mon& mon)
{
  bool blocked[MAP_W][MAP_H];
  CellCheck::BlocksActor cellcheck(mon, true);

  const Pos& monPos = mon.pos;
  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      const Pos p(monPos.x + dx, monPos.y + dy);
      blocked[p.x][p.y] = cellcheck.check(Map::cells[p.x][p.y]);
    }
  }

  for (Actor* actor : GameTime::actors_)
  {
    const Pos& p = actor->pos;
    blocked[p.x][p.y] = true;
  }
  for (Mob* mob : GameTime::mobs_)
  {
    const Pos& p = mob->getPos();
    blocked[p.x][p.y] = cellcheck.check(*mob);
  }

  const Rect areaAllowed(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

  //First, try the same direction as last travelled
  const Dir lastDirTravelled = mon.lastDirTravelled_;
  if (lastDirTravelled != Dir::center)
  {
    const Pos targetCell(monPos + DirUtils::getOffset(lastDirTravelled));
    if (!blocked[targetCell.x][targetCell.y] &&
        Utils::isPosInside(targetCell, areaAllowed))
    {
      return lastDirTravelled;
    }
  }

  //Attempt to find a random non-blocked adjacent cell
  vector<Dir> dirBucket;
  dirBucket.clear();
  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      if (dx != 0 || dy != 0)
      {
        const Pos offset(dx, dy);
        const Pos targetCell(monPos + offset);
        if (!blocked[targetCell.x][targetCell.y] &&
            Utils::isPosInside(targetCell, areaAllowed))
        {
          dirBucket.push_back(DirUtils::getDir(offset));
        }
      }
    }
  }

  const int NR_ELEMENTS = dirBucket.size();
  if (NR_ELEMENTS == 0)
  {
    return Dir::center;
  }
  else
  {
    return dirBucket[Rnd::range(0, NR_ELEMENTS - 1)];
  }
}

} //namespace

bool moveToRandomAdjCell(Mon& mon)
{
  if (mon.isAlive())
  {
    if (mon.isRoamingAllowed_ || mon.awareCounter_ > 0)
    {
      const Dir dir = getDirToRndAdjFreeCell(mon);
      if (dir != Dir::center)
      {
        mon.moveDir(dir);
        return true;
      }
    }
  }
  return false;
}

bool moveToTgtSimple(Mon& mon)
{
  if (mon.isAlive() && mon.tgt_)
  {
    if (mon.awareCounter_ > 0 || Map::player->isLeaderOf(&mon))
    {
      const Pos offset  = mon.tgt_->pos - mon.pos;
      const Pos signs   = offset.getSigns();
      bool blocked[MAP_W][MAP_H];
      MapParse::run(CellCheck::BlocksActor(mon, true), blocked);
      const Pos newPos(mon.pos + signs);
      if (!blocked[newPos.x][newPos.y])
      {
        mon.moveDir(DirUtils::getDir(signs));
        return true;
      }
    }
  }
  return false;
}

bool stepPath(Mon& mon, vector<Pos>& path)
{
  if (mon.isAlive())
  {
    if (!path.empty())
    {
      const Pos delta = path.back() - mon.pos;
      mon.moveDir(DirUtils::getDir(delta));
      return true;
    }
  }
  return false;
}

bool stepToLairIfLos(Mon& mon, const Pos& lairCell)
{
  if (mon.isAlive())
  {
    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksLos(), blocked);
    const bool HAS_LOS_TO_LAIR =
      Fov::checkCell(blocked, lairCell, mon.pos, true);

    if (HAS_LOS_TO_LAIR)
    {
      Pos delta = lairCell - mon.pos;

      delta.x = delta.x == 0 ? 0 : (delta.x > 0 ? 1 : -1);
      delta.y = delta.y == 0 ? 0 : (delta.y > 0 ? 1 : -1);
      const Pos newPos = mon.pos + delta;

      MapParse::run(CellCheck::BlocksLos(), blocked);
      if (blocked[newPos.x][newPos.y])
      {
        return false;
      }
      else
      {
        mon.moveDir(DirUtils::getDir(delta));
        return true;
      }
    }
  }

  return false;
}

} //Action

namespace Info
{

bool lookBecomePlayerAware(Mon& mon)
{
  if (mon.isAlive())
  {

    const bool WAS_AWARE_BEFORE = mon.awareCounter_ > 0;

    vector<Actor*> seenFoes;
    mon.getSeenFoes(seenFoes);

    if (!seenFoes.empty() && WAS_AWARE_BEFORE)
    {
      mon.becomeAware(false);
      return false;
    }

    for (Actor* actor : seenFoes)
    {
      if (actor == Map::player)
      {
        if (mon.isSpottingHiddenActor(*actor))
        {
          mon.becomeAware(true);
          if (WAS_AWARE_BEFORE)
          {
            return false;
          }
          else
          {
            GameTime::tick();
            return true;
          }
        }
      }
      else
      {
        mon.becomeAware(false);
        if (WAS_AWARE_BEFORE)
        {
          return false;
        }
        else
        {
          GameTime::tick();
          return true;
        }
      }
    }
  }
  return false;
}

void setPathToLairIfNoLos(Mon& mon, vector<Pos>& path,
                          const Pos& lairCell)
{
  if (mon.isAlive())
  {

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksLos(), blocked);

    if (Fov::checkCell(blocked, lairCell, mon.pos, true))
    {
      path.clear();
      return;
    }

    MapParse::run(CellCheck::BlocksActor(mon, false), blocked);

    MapParse::run(CellCheck::LivingActorsAdjToPos(mon.pos),
                    blocked, MapParseMode::append);

    PathFind::run(mon.pos, lairCell, blocked, path);
    return;
  }

  path.clear();
}

void setPathToLeaderIfNoLosToleader(Mon& mon, vector<Pos>& path)
{
  if (mon.isAlive())
  {
    Actor* leader = mon.leader_;
    if (leader)
    {
      if (leader->isAlive())
      {
        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksLos(), blocked);

        if (Fov::checkCell(blocked, leader->pos, mon.pos, true))
        {
          path.clear();
          return;
        }

        MapParse::run(CellCheck::BlocksActor(mon, false), blocked);

        MapParse::run(CellCheck::LivingActorsAdjToPos(mon.pos),
                        blocked, MapParseMode::append);

        PathFind::run(mon.pos, leader->pos, blocked, path);
        return;
      }
    }
  }

  path.clear();
}

void setPathToPlayerIfAware(Mon& mon, vector<Pos>& path)
{
  if (mon.isAlive())
  {
    if (mon.awareCounter_ > 0)
    {

      bool blocked[MAP_W][MAP_H];
      Utils::resetArray(blocked, false);

      bool props[endOfPropIds];
      mon.getPropHandler().getPropIds(props);

      for (int y = 1; y < MAP_H - 1; ++y)
      {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
          const auto* const f = Map::cells[x][y].rigid;
          if (!f->canMove(props))
          {
            if (f->getId() == FeatureId::door)
            {
              const Door* const door = static_cast<const Door*>(f);

              const ActorDataT& d = mon.getData();

              if ((!d.canOpenDoors && !d.canBashDoors) || door->isHandledExternally())
              {
                blocked[x][y] = true;
              }
            }
            else //Not a door
            {
              blocked[x][y] = true;
            }
          }
        }
      }

      //Append living adjacent actors to the blocking array
      MapParse::run(CellCheck::LivingActorsAdjToPos(mon.pos),
                      blocked, MapParseMode::append);

      PathFind::run(mon.pos, Map::player->pos, blocked, path);
    }
    else
    {
      path.clear();
    }
  }
  else
  {
    path.clear();
  }
}

void setSpecialBlockedCells(Mon& mon, bool a[MAP_W][MAP_H])
{
  (void)mon;
  (void)a;
}

} //Info

} //Ai

