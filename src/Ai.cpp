#include "Ai.h"

#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Utils.h"
#include "FeatureDoor.h"
#include "ActorMonster.h"
#include "LineCalc.h"
#include "MapParsing.h"
#include "GameTime.h"
#include "Fov.h"

using namespace std;

namespace Ai {

namespace Action {

bool castRandomSpellIfAware(Monster* monsterActing) {
  if(monsterActing->deadState == ActorDeadState::alive) {
    if(
      monsterActing->awareOfPlayerCounter_ > 0 &&
      monsterActing->spellCoolDownCurrent == 0) {

      if(monsterActing->getPropHandler().allowRead(false) == false) {
        return false;
      }

      if(monsterActing->spellsKnown.empty() == false) {
        vector<Spell*> spellCandidates = monsterActing->spellsKnown;

        while(spellCandidates.empty() == false) {
          const unsigned int ELEMENT =
            Rnd::range(0, spellCandidates.size() - 1);

          Spell* const spell = spellCandidates.at(ELEMENT);

          if(spell->isGoodForMonsterToCastNow(monsterActing)) {
            const int CUR_SPI = monsterActing->getSpi();
            const int SPELL_MAX_SPI =
              spell->getSpiCost(false, monsterActing).upper;

            // Cast spell if max spirit cost is lower than current spirit,
            if(SPELL_MAX_SPI < CUR_SPI) {
              spell->cast(monsterActing, true);
              return true;
            }

            const int CUR_HP  = monsterActing->getHp();
            const int MAX_HP  = monsterActing->getHpMax(true);

            // Cast spell with a certain chance if HP is low.
            if(CUR_HP < MAX_HP / 3 && Rnd::percentile() <= 5) {
              if(Map::player->isSeeingActor(*monsterActing, NULL)) {
                Log::addMsg(monsterActing->getNameThe() + " looks desperate.");
              }
              spell->cast(monsterActing, true);
              return true;
            }
            return false;
          } else {
            spellCandidates.erase(spellCandidates.begin() + ELEMENT);
          }
        }
      }
    }
  }
  return false;
}

bool handleClosedBlockingDoor(Monster& monster, vector<Pos> path) {
  if(monster.deadState == ActorDeadState::alive && path.empty() == false) {
    const Pos& p = path.back();
    Feature* const f = Map::cells[p.x][p.y].featureStatic;
    if(f->getId() == FeatureId::door) {
      Door* const door = dynamic_cast<Door*>(f);
      vector<PropId> props;
      monster.getPropHandler().getAllActivePropIds(props);
      if(door->canMove(props) == false) {
        if(door->isStuck() == false) {
          if(monster.getData().canOpenDoors) {
            door->tryOpen(&monster);
            return true;
          } else if(monster.getData().canBashDoors) {
            door->bash(monster);
            return true;
          }
        } else if(monster.getData().canBashDoors) {
          door->bash(monster);
          return true;
        }
      }
    }
  }
  return false;
}

bool handleInventory(Monster* monster) {
  (void)monster;
  return false;
}

//Helper functions for makeRoomForFriend()
namespace {

//Check if acting monster is on a line between player and other monster
bool checkIfBlockingMon(const Pos& pos, Monster* other) {
  vector<Pos> line;
  LineCalc::calcNewLine(other->pos, Map::player->pos, true, 9999, false, line);

  for(const Pos & posInLine : line) {if(posInLine == pos) {return true;}}
  return false;
}

//Returns all free positions around the acting monster that is further
//from the player than the monster's current position
void getMoveCandidates(Monster& self, vector<Pos>& dirsToMake) {

  dirsToMake.resize(0);

  const int PLAYER_X = Map::player->pos.x;
  const int PLAYER_Y = Map::player->pos.y;
  const int OLD_X = self.pos.x;
  const int OLD_Y = self.pos.y;

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksActor(self, true), blockers);

  for(int x = -1; x <= 1; x++) {
    for(int y = -1; y <= 1; y++) {
      if(x != OLD_X || y != OLD_Y) {
        const int NEW_X = OLD_X + x;
        const int NEW_Y = OLD_Y + y;
        const int DIST_CUR = Utils::kingDist(OLD_X, OLD_Y, PLAYER_X, PLAYER_Y);
        const int DIST_NEW = Utils::kingDist(NEW_X, NEW_Y, PLAYER_X, PLAYER_Y);

        if(DIST_NEW <= DIST_CUR) {
          if(blockers[NEW_X][NEW_Y] == false) {
            dirsToMake.push_back(Pos(NEW_X, NEW_Y));
          }
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
bool isAdjAndWithoutVision(const Monster& self, Monster& other,
                           bool visionBlockers[MAP_W][MAP_H]) {
  //If the pal is next to me
  if(Utils::isPosAdj(self.pos, other.pos, false)) {
    //If pal does not see player
    if(other.isSeeingActor(*Map::player, visionBlockers) == false) {
      return true;
    }
  }
  return false;
}

} //namespace

bool makeRoomForFriend(Monster& monster) {
  if(monster.deadState == ActorDeadState::alive) {

    bool visionBlockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), visionBlockers);

    if(monster.isSeeingActor(*Map::player, visionBlockers)) {

      //Loop through all actors
      for(Actor * actor : GameTime::actors_) {
        if(actor != Map::player && actor != &monster) {
          if(actor->deadState == ActorDeadState::alive) {
            Monster* otherMonster = dynamic_cast<Monster*>(actor);

            bool isOtherAdjWithoutVision =
              isAdjAndWithoutVision(
                monster, *otherMonster, visionBlockers);

            //Other monster sees the player, or it's a neighbour that
            //does not see the player?
            if(
              otherMonster->isSeeingActor(
                *Map::player, visionBlockers) ||
              isOtherAdjWithoutVision) {

              // If we are indeed blocking a pal, check every neighbouring
              //cell that is at equal or closer distance to the player,
              //to check whether they are fine.

              // TODO Vision must be checked from the cell candidates!

              // TODO If several good candidates are found,
              //result should be picked from them at random

              if(
                checkIfBlockingMon(monster.pos, otherMonster) ||
                isOtherAdjWithoutVision) {

                // Get a list of neighbouring free cells
                vector<Pos> candidates;
                getMoveCandidates(monster, candidates);

                // Sort the list by closeness to player
                IsCloserToOrigin sorter(Map::player->pos);
                sort(candidates.begin(), candidates.end(), sorter);

                // Test the candidate cells until one is found that
                //is not also blocking someone.
                const int NR_CANDIDATES = candidates.size();
                for(int ii = 0; ii < NR_CANDIDATES; ii++) {

                  bool isGoodCandidateFound = true;

                  for(Actor * actor2 : GameTime::actors_) {
                    if(actor2 != Map::player && actor2 != &monster) {
                      otherMonster = dynamic_cast<Monster*>(actor2);
                      if(
                        otherMonster->isSeeingActor(
                          *Map::player, visionBlockers)) {
                        if(
                          checkIfBlockingMon(
                            candidates.at(ii), otherMonster)) {
                          isGoodCandidateFound = false;
                          break;
                        }
                      }
                    }
                  }

                  if(isGoodCandidateFound) {
                    const Pos offset = candidates.at(ii) - monster.pos;
                    monster.moveDir(DirUtils::getDir(offset));
                    return true;
                  }
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

//Helper functions for moveToRandomAdjacentCell()
namespace {

Dir getDirToRndAdjFreeCell(Monster& monster) {

  //This is slightly hacky (a bit too coupled to implementation details),
  //but it optimizes this (very frequently used) function
  bool blockers[MAP_W][MAP_H];
  CellPred::BlocksActor cellPred(monster, true);

  const Pos& monsterPos = monster.pos;
  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      const Pos p(monsterPos.x + dx, monsterPos.y + dy);
      blockers[p.x][p.y] = cellPred.check(Map::cells[p.x][p.y]);
    }
  }

  for(Actor * actor : GameTime::actors_) {
    const Pos& p = actor->pos;
    blockers[p.x][p.y] = true;
  }
  for(FeatureMob * mob : GameTime::featureMobs_) {
    const Pos& p = mob->getPos();
    blockers[p.x][p.y] = cellPred.check(*mob);
  }

  const Rect areaAllowed(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

  //First, try the same direction as last travelled
  const Dir lastDirTravelled = monster.lastDirTravelled_;
  if(lastDirTravelled != Dir::center) {
    const Pos targetCell(monsterPos + DirUtils::getOffset(lastDirTravelled));
    if(
      blockers[targetCell.x][targetCell.y] == false &&
      Utils::isPosInside(targetCell, areaAllowed)) {
      return lastDirTravelled;
    }
  }

  //Attempt to find a random non-blocked adjacent cell
  vector<Dir> dirCandidates;
  dirCandidates.resize(0);
  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      if(dx != 0 || dy != 0) {
        const Pos offset(dx, dy);
        const Pos targetCell(monsterPos + offset);
        if(
          blockers[targetCell.x][targetCell.y] == false &&
          Utils::isPosInside(targetCell, areaAllowed)) {
          dirCandidates.push_back(DirUtils::getDir(offset));
        }
      }
    }
  }

  const int NR_ELEMENTS = dirCandidates.size();
  if(NR_ELEMENTS == 0) {
    return Dir::center;
  } else {
    return dirCandidates.at(Rnd::range(0, NR_ELEMENTS - 1));
  }
}

} //namespace

bool moveToRandomAdjacentCell(Monster& monster) {
  if(monster.deadState == ActorDeadState::alive) {
    if(
      monster.isRoamingAllowed_ == true ||
      monster.awareOfPlayerCounter_ > 0) {

      const Dir dir = getDirToRndAdjFreeCell(monster);
      if(dir != Dir::center) {
        monster.moveDir(dir);
        return true;
      }
    }
  }
  return false;
}

bool moveTowardsTargetSimple(Monster& monster) {
  if(monster.deadState == ActorDeadState::alive) {
    if(monster.target != NULL) {
      if(
        monster.awareOfPlayerCounter_ > 0 ||
        monster.leader == Map::player) {
        Pos result;
        Pos offset = monster.target->pos - monster.pos;
        offset.x = offset.x == 0 ? 0 : (offset.x > 0 ? 1 : -1);
        offset.y = offset.y == 0 ? 0 : (offset.y > 0 ? 1 : -1);
        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(monster, true),
                        blockers);
        const Pos newPos(monster.pos + offset);
        if(blockers[newPos.x][newPos.y] == false) {
          monster.moveDir(DirUtils::getDir(offset));
          return true;
        } else {
          return false;
        }
      }
    }
  }
  return false;
}

bool stepPath(Monster& monster, vector<Pos>& path) {
  if(monster.deadState == ActorDeadState::alive) {
    if(path.empty() == false) {
      const Pos delta = path.back() - monster.pos;
      monster.moveDir(DirUtils::getDir(delta));
      return true;
    }
  }
  return false;
}

bool stepToLairIfLos(Monster& monster, const Pos& lairCell) {
  if(monster.deadState == ActorDeadState::alive) {
    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blockers);
    const bool HAS_LOS_TO_LAIR =
      Fov::checkCell(blockers, lairCell, monster.pos, true);

    if(HAS_LOS_TO_LAIR) {
      Pos delta = lairCell - monster.pos;

      delta.x = delta.x == 0 ? 0 : (delta.x > 0 ? 1 : -1);
      delta.y = delta.y == 0 ? 0 : (delta.y > 0 ? 1 : -1);
      const Pos newPos = monster.pos + delta;

      MapParse::parse(CellPred::BlocksVision(), blockers);
      if(blockers[newPos.x][newPos.y]) {
        return false;
      } else {
        monster.moveDir(DirUtils::getDir(delta));
        return true;
      }
    }
  }

  return false;
}

} //Action

namespace Info {

bool lookBecomePlayerAware(Monster& monster) {
  if(monster.deadState == ActorDeadState::alive) {

    const bool WAS_AWARE_BEFORE = monster.awareOfPlayerCounter_ > 0;

    vector<Actor*> spottedEnemies;
    monster.getSpottedEnemies(spottedEnemies);

    if(spottedEnemies.empty() == false && WAS_AWARE_BEFORE) {
      monster.becomeAware(false);
      return false;
    }

    for(Actor * actor : spottedEnemies) {
      if(actor == Map::player) {
        if(monster.isSpottingHiddenActor(*actor)) {
          monster.becomeAware(true);
          if(WAS_AWARE_BEFORE) {
            return false;
          } else {
            GameTime::actorDidAct();
            return true;
          }
        }
      } else {
        monster.becomeAware(false);
        if(WAS_AWARE_BEFORE) {
          return false;
        } else {
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void setPathToLairIfNoLos(Monster& monster, vector<Pos>& path,
                          const Pos& lairCell) {
  if(monster.deadState == ActorDeadState::alive) {

    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blockers);

    if(Fov::checkCell(blockers, lairCell, monster.pos, true)) {
      path.resize(0);
      return;
    }

    MapParse::parse(CellPred::BlocksActor(monster, false), blockers);

    MapParse::parse(CellPred::LivingActorsAdjToPos(monster.pos),
                    blockers, MapParseWriteRule::writeOnlyTrue);

    PathFind::run(monster.pos, lairCell, blockers, path);
    return;
  }

  path.resize(0);
}

void setPathToLeaderIfNoLosToleader(Monster& monster, vector<Pos>& path) {
  if(monster.deadState == ActorDeadState::alive) {
    Actor* leader = monster.leader;
    if(leader != NULL) {
      if(leader->deadState == ActorDeadState::alive) {
        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blockers);

        if(
          Fov::checkCell(blockers, leader->pos, monster.pos, true)) {
          path.resize(0);
          return;
        }

        MapParse::parse(CellPred::BlocksActor(monster, false), blockers);

        MapParse::parse(CellPred::LivingActorsAdjToPos(monster.pos),
                        blockers, MapParseWriteRule::writeOnlyTrue);

        PathFind::run(monster.pos, leader->pos, blockers, path);
        return;
      }
    }
  }

  path.resize(0);
}

void setPathToPlayerIfAware(Monster& monster, vector<Pos>& path) {
  if(monster.deadState == ActorDeadState::alive) {
    if(monster.awareOfPlayerCounter_ > 0) {

      bool blockers[MAP_W][MAP_H];
      Utils::resetArray(blockers, false);

      vector<PropId> props;
      monster.getPropHandler().getAllActivePropIds(props);

      for(int y = 1; y < MAP_H - 1; y++) {
        for(int x = 1; x < MAP_W - 1; x++) {
          const Feature* const f = Map::cells[x][y].featureStatic;
          if(f->canMove(props) == false) {

            if(f->getId() == FeatureId::door) {

              const Door* const door = dynamic_cast<const Door*>(f);

              const ActorDataT& d = monster.getData();

              if(
                (d.canOpenDoors == false && d.canBashDoors == false) ||
                door->isOpenedAndClosedExternally()) {
                blockers[x][y] = true;
              }

            } else {
              blockers[x][y] = true;
            }
          }
        }
      }

      //Append living adjacent actors to the blocking array
      MapParse::parse(CellPred::LivingActorsAdjToPos(monster.pos),
                      blockers, MapParseWriteRule::writeOnlyTrue);

      PathFind::run(monster.pos, Map::player->pos, blockers, path);
    } else {
      path.resize(0);
    }
  } else {
    path.resize(0);
  }
}

void setSpecialBlockedCells(Monster& monster, bool a[MAP_W][MAP_H]) {
  (void)monster;
  (void)a;
}

} //Info

} //Ai

