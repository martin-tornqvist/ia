#ifndef AI_SET_PATH_TO_LAIR_IF_NO_LOS_TO_LAIR_H
#define AI_SET_PATH_TO_LAIR_IF_NO_LOS_TO_LAIR_H

#include "Engine.h"

class AI_setPathToLairIfNoLosToLair {
public:
  static void learn(Monster* monster, vector<coord>* path, const coord& lairCell, Engine* engine) {

    if(monster->deadState == actorDeadState_alive) {

      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      engine->mapTests->makeVisionBlockerArray(blockers);

      if(engine->fov->checkOneCell(blockers, lairCell.x, lairCell.y, monster->pos.x, monster->pos.y, true)) {
        path->resize(0);
        return;
      }

      engine->mapTests->makeMoveBlockerArrayFeaturesOnly(monster, blockers);
      engine->mapTests->addAdjacentLivingActorsToBlockerArray(monster->pos, blockers);
      *path = engine->pathfinder->findPath(monster->pos, blockers, lairCell);
      return;
    }

    path->resize(0);

  }
private:
};

#endif
