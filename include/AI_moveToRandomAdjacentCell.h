#ifndef AI_MOVE_TO_RANDOM_ADJ
#define AI_MOVE_TO_RANDOM_ADJ

#include "Engine.h"
#include "Actor.h"
#include "CommonData.h"

namespace {

Dir getDirToRndAdjFreeCell(Monster& monster, Engine& engine) {

  //This is slightly hacky (a bit too coupled to implementation details),
  //but it optimizes this (very frequently used) function
  bool blockers[MAP_W][MAP_H];
  CellPred::BlocksActor cellPred(monster, true, engine);

  const Pos& monsterPos = monster.pos;
  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      const Pos p(monsterPos.x + dx, monsterPos.y + dy);
      blockers[p.x][p.y] = cellPred.check(engine.map->cells[p.x][p.y]);
    }
  }

  for(Actor* actor : engine.gameTime->actors_) {
    const Pos& p = actor->pos;
    blockers[p.x][p.y] = true;
  }
  for(FeatureMob* mob : engine.gameTime->featureMobs_) {
    const Pos& p = mob->getPos();
    blockers[p.x][p.y] = cellPred.check(*mob);
  }

  const Rect areaAllowed(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

  //First, try the same direction as last travelled
  const Dir lastDirTravelled = monster.lastDirTravelled_;
  DirConverter dirConv;
  if(lastDirTravelled != dirCenter) {
    const Pos targetCell(monsterPos + dirConv.getOffset(lastDirTravelled));
    if(
      blockers[targetCell.x][targetCell.y] == false &&
      engine.basicUtils->isPosInside(targetCell, areaAllowed)) {
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
          engine.basicUtils->isPosInside(targetCell, areaAllowed)) {
          dirCandidates.push_back(dirConv.getDir(offset));
        }
      }
    }
  }

  const int NR_ELEMENTS = dirCandidates.size();
  if(NR_ELEMENTS == 0) {
    return dirCenter;
  } else {
    return dirCandidates.at(engine.dice.range(0, NR_ELEMENTS - 1));
  }
}

} //namespace

namespace AI_moveToRandomAdjacentCell {

static bool action(Monster& monster, Engine& engine) {
  if(monster.deadState == actorDeadState_alive) {
    if(
      monster.isRoamingAllowed_ == true ||
      monster.awareOfPlayerCounter_ > 0) {

      const Dir dir = getDirToRndAdjFreeCell(monster, engine);
      if(dir != dirCenter) {
        monster.moveDir(dir);
        return true;
      }
    }
  }
  return false;
}

}

#endif
