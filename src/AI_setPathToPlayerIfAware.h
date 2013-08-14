#ifndef AI_SET_PATH_TO_PLAYER_IF_AWARE_H
#define AI_SET_PATH_TO_PLAYER_IF_AWARE_H

#include "Engine.h"
#include "Pathfinding.h"

class AI_setPathToPlayerIfAware {
public:
  static void learn(Monster* monster, vector<Pos>* path, Engine* engine) {
    if(monster->deadState == actorDeadState_alive) {
      if(monster->playerAwarenessCounter > 0) {

        const ActorDef* const d = monster->getDef();
        const bool CONSIDER_NORMAL_DOORS_FREE = d->canOpenDoors || d->canBashDoors;

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        engine->basicUtils->resetBoolArray(blockers, false);
        for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
          for(int x = 1; x < MAP_X_CELLS - 1; x++) {
            if(engine->map->featuresStatic[x][y]->isMovePassable(monster) == false) {

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

//        FeatureMob* f = NULL;
//        const unsigned int FEATURE_MOBS_SIZE = engine->gameTime->getFeatureMobsSize();
//        for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//          f = engine->gameTime->getFeatureMobAt(i);
//          if(blockers[f->getX()][f->getY()] == false) {
//            blockers[f->getX()][f->getY()] = f->isShootPassable() == false;
//          }
//        }

        engine->mapTests->addAdjacentLivingActorsToBlockerArray(monster->pos, blockers);

        *path = engine->pathfinder->findPath(monster->pos, blockers, engine->player->pos);
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

