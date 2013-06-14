#ifndef AI_MOVE_TO_RANDOM_ADJ
#define AI_MOVE_TO_RANDOM_ADJ

#include "Engine.h"
#include "Actor.h"
#include "CommonTypes.h"

class AI_moveToRandomAdjacentCell {
public:

  static bool action(Monster* const monster, Engine* const engine) {
    if(monster->deadState == actorDeadState_alive) {
      if(monster->isRoamingAllowed == true || monster->playerAwarenessCounter > 0) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        engine->mapTests->makeMoveBlockerArray(monster, blockers);

        const coord result = getRandomAdjacentFreeCell(monster, blockers, engine);
        if(result != monster->pos) {
          monster->moveToCell(result);
          return true;
        }
      }
    }
    return false;
  }

private:
  static coord getRandomAdjacentFreeCell(Monster* const monster, bool blockers[MAP_X_CELLS][MAP_Y_CELLS], Engine* const engine) {

    const Rect areaAllowed(coord(1, 1), coord(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

    //First, try the same direction as last travelled
    if(monster->lastDirectionTraveled != coord(0, 0)) {
      const coord result(monster->pos + monster->lastDirectionTraveled);
      if(blockers[result.x][result.y] == false && engine->mapTests->isCellInside(result, areaAllowed)) {
        return result;
      }
    }

    //Attempt to find  a random non-blocked adjacent cell
    for(int i = 0; i < 16; i++) {
      const int DX = engine->dice.getInRange(-1, 1);
      const int DY = engine->dice.getInRange(-1, 1);
      const coord result(monster->pos + coord(DX, DY));

      if(blockers[result.x][result.y] == false && engine->mapTests->isCellInside(result, areaAllowed)) {
        return result;
      }
    }

    //If this point reached, no cell was found
    return monster->pos;
  }
};

#endif
