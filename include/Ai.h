#ifndef AI_H
#define AI_H

#include <vector>

#include "CmnTypes.h"
#include "CmnData.h"

class Mon;

namespace Ai
{

namespace Action
{

bool castRandomSpellIfAware(Mon& mon);

bool handleClosedBlockingDoor(Mon& mon, std::vector<Pos> path);

bool handleInventory(Mon& mon);

bool makeRoomForFriend(Mon& mon);

bool moveToRandomAdjCell(Mon& mon);

bool moveToTgtSimple(Mon& mon);

bool stepPath(Mon& mon, std::vector<Pos>& path);

bool stepToLairIfLos(Mon& mon, const Pos& lairCell);

} //Action

namespace Info
{

bool lookBecomePlayerAware(Mon& mon);

void setPathToLairIfNoLos(Mon& mon, std::vector<Pos>& path, const Pos& lairCell);

void setPathToLeaderIfNoLosToleader(Mon& mon, std::vector<Pos>& path);

void setPathToPlayerIfAware(Mon& mon, std::vector<Pos>& path);

void setSpecialBlockedCells(Mon& mon, bool a[MAP_W][MAP_H]);

} //Info

} //Ai

#endif
