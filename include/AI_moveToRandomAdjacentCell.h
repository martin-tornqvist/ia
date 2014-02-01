#ifndef AI_MOVE_TO_RANDOM_ADJ
#define AI_MOVE_TO_RANDOM_ADJ

#include "Engine.h"
#include "Actor.h"
#include "CommonData.h"

namespace {

Dir_t getDirToRndAdjFreeCell(Monster& monster, Engine& engine) {

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

  const int NR_ACTORS   = engine.gameTime->getNrActors();
  const int NR_FEATURES = engine.gameTime->getNrFeatureMobs();

  for(int i = 0; i < NR_ACTORS; i++) {
    const Pos& pos = engine.gameTime->getActorAtElement(i).pos;
    blockers[pos.x][pos.y] = true;
  }
  for(int i = 0; i < NR_FEATURES; i++) {
    const FeatureMob& m = engine.gameTime->getFeatureMobAtElement(i);
    const Pos& p = m.getPos();
    blockers[p.x][p.y] = cellPred.check(m);
  }

  const Rect areaAllowed(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

  //First, try the same direction as last travelled
  const Dir_t lastDirTravelled = monster.lastDirTravelled;
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
  vector<Dir_t> dirCandidates;
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
      monster.isRoamingAllowed == true ||
      monster.awareOfPlayerCounter > 0) {

      const Dir_t dir = getDirToRndAdjFreeCell(monster, engine);
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
