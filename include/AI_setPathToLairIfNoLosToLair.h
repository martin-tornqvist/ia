#ifndef AI_SET_PATH_TO_LAIR_IF_NO_LOS_TO_LAIR_H
#define AI_SET_PATH_TO_LAIR_IF_NO_LOS_TO_LAIR_H

#include "Engine.h"

class AI_setPathToLairIfNoLosToLair {
public:
  static void learn(Monster& monster, vector<Pos>* path,
                    const Pos& lairCell, Engine* engine) {

    if(monster.deadState == actorDeadState_alive) {

      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      MapParser::parse(CellPredBlocksVision(engine), blockers);

      if(engine->fov->checkCell(blockers, lairCell, monster.pos, true)) {
        path->resize(0);
        return;
      }

      MapParser::parse(
        CellPredBlocksBodyType(monster.getBodyType(), false, engine),
        blockers);
      MapParser::parse(
        CellPredLivingActorsAdjToPos(monster.pos, engine),
        blockers, mapParseWriteOnlyTrue);
      *path = engine->pathfinder->findPath(monster.pos, blockers, lairCell);
      return;
    }

    path->resize(0);

  }
private:
};

#endif
