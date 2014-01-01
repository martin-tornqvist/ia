#ifndef AI_MOVE_TO_RANDOM_ADJ
#define AI_MOVE_TO_RANDOM_ADJ

#include "Engine.h"
#include "Actor.h"
#include "CommonData.h"

class AI_moveToRandomAdjacentCell {
public:

  static bool action(Monster& monster, Engine& engine) {
    if(monster.deadState == actorDeadState_alive) {
      if(
        monster.isRoamingAllowed == true ||
        monster.playerAwarenessCounter > 0) {

        bool blockers[MAP_W][MAP_H];
        MapParser::parse(
          CellPredBlocksBodyType(monster.getBodyType(), true, engine),
          blockers);

        const Pos offset =
          getOffsetToRandomAdjacentFreeCell(monster, blockers, engine);
        const Dir_t dir = DirConverter().getDir(offset);
        if(dir != dirCenter) {
          monster.moveDir(dir);
          return true;
        }
      }
    }
    return false;
  }

private:
  static Pos getOffsetToRandomAdjacentFreeCell(
    const Monster& monster, bool blockers[MAP_W][MAP_H],
    Engine& engine) {

    const Rect areaAllowed(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

    //First, try the same direction as last travelled
    if(monster.lastDirTraveled != dirCenter) {
      const Pos offset =
        DirConverter().getOffset(monster.lastDirTraveled);
      const Pos targetCell(monster.pos + offset);
      if(
        blockers[targetCell.x][targetCell.y] == false &&
        engine.basicUtils->isPosInside(targetCell, areaAllowed)) {
        return offset;
      }
    }

    //Attempt to find a random non-blocked adjacent cell
    vector<Pos> offsetCandidates;
    offsetCandidates.resize(0);
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        if(dx != 0 || dy != 0) {
          const Pos offset(dx, dy);
          const Pos targetCell(monster.pos + offset);
          if(
            blockers[targetCell.x][targetCell.y] == false &&
            engine.basicUtils->isPosInside(targetCell, areaAllowed)) {
            offsetCandidates.push_back(offset);
          }
        }
      }
    }

    const int NR_ELEMENTS = offsetCandidates.size();
    if(NR_ELEMENTS == 0) {
      return Pos(0, 0);
    } else {
      return offsetCandidates.at(engine.dice.range(0, NR_ELEMENTS - 1));
    }
  }
};

#endif
