#ifndef AI_SET_PATH_TO_PLAYER_IF_AWARE_H
#define AI_SET_PATH_TO_PLAYER_IF_AWARE_H

#include "Engine.h"
#include "MapParsing.h"

class AI_setPathToPlayerIfAware {
public:
  static void learn(Monster& monster, vector<Pos>& path, Engine& engine) {
    if(monster.deadState == actorDeadState_alive) {
      if(monster.playerAwarenessCounter > 0) {

        const ActorData& d = monster.getData();
        const bool CONSIDER_NORMAL_DOORS_FREE =
          d.canOpenDoors || d.canBashDoors;

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        engine.basicUtils->resetArray(blockers, false);
        for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
          for(int x = 1; x < MAP_X_CELLS - 1; x++) {
            const Feature* const f = engine.map->cells[x][y].featureStatic;
            if(f->isMovePassable(&monster) == false) {

              if(f->getId() == feature_door) {

                const Door* const door = dynamic_cast<const Door*>(f);

                if(
                  CONSIDER_NORMAL_DOORS_FREE == false ||
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
        MapParser::parse(CellPredLivingActorsAdjToPos(monster.pos, engine),
                         blockers, mapParseWriteOnlyTrue);

        engine.pathFinder->run(
          monster.pos, engine.player->pos, blockers, path);
      } else {
        path.resize(0);
      }
    } else {
      path.resize(0);
    }
  }

private:
};

#endif

