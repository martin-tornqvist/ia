#ifndef AI_SET_PATH_TO_PLAYER_IF_AWARE_H
#define AI_SET_PATH_TO_PLAYER_IF_AWARE_H

#include "Engine.h"
#include "Pathfinding.h"

class AI_setPathToPlayerIfAware {
public:
  static void learn(Monster& monster, vector<Pos>* path, Engine* engine) {
    if(monster.deadState == actorDeadState_alive) {
      if(monster.playerAwarenessCounter > 0) {

        const ActorData* const d = monster.getData();
        const bool CONSIDER_NORMAL_DOORS_FREE =
          d->canOpenDoors || d->canBashDoors;

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        engine->basicUtils->resetBoolArray(blockers, false);
        for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
          for(int x = 1; x < MAP_X_CELLS - 1; x++) {
              Feature* const f = engine->map->featuresStatic[x][y];
            if(f->isMovePassable(&monster) == false) {

              if(engine->map->featuresStatic[x][y]->getId() == feature_door) {

                Door* const door = dynamic_cast<Door*>(engine->map->featuresStatic[x][y]);

                if(CONSIDER_NORMAL_DOORS_FREE == false || door->isOpenedAndClosedExternally()) {
                  blockers[x][y] = true;
                }

              } else {
                blockers[x][y] = true;
              }
            }
          }
        }

        engine->mapTests->addAdjLivingActorsToBlockerArray(monster.pos, blockers);

        *path = engine->pathfinder->findPath(monster.pos, blockers, engine->player->pos);
      } else {
        path->resize(0);
      }
    } else {
      path->resize(0);
    }
  }

private:
};

#endif

