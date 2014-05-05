#ifndef AI_H
#define AI_H

#include <vector>

#include "CmnTypes.h"
#include "CmnData.h"

class Monster;

namespace Ai {

namespace Action {

bool castRandomSpellIfAware(Monster* monsterActing);

bool handleClosedBlockingDoor(Monster& monster, std::vector<Pos> path);

bool handleInventory(Monster* monster);

bool makeRoomForFriend(Monster& monster);

bool moveToRandomAdjacentCell(Monster& monster);

bool moveTowardsTargetSimple(Monster& monster);

bool stepPath(Monster& monster, std::vector<Pos>& path);

bool stepToLairIfLos(Monster& monster, const Pos& lairCell);

} //Action

namespace Info {

bool lookBecomePlayerAware(Monster& monster);

void setPathToLairIfNoLos(Monster& monster, std::vector<Pos>& path,
                          const Pos& lairCell);

void setPathToLeaderIfNoLosToleader(Monster& monster, std::vector<Pos>& path);

void setPathToPlayerIfAware(Monster& monster, std::vector<Pos>& path);

void setSpecialBlockedCells(Monster& monster, bool a[MAP_W][MAP_H]);

} //Info

} //Ai

#endif
