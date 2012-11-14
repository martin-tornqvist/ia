#ifndef AI_MAKE_ROOM_FOR_FRIEND_H
#define AI_MAKE_ROOM_FOR_FRIEND_H

#include <algorithm>

/*===========================================================================++
 Purpose:
 Check if monster blocks the path to the player for another monster. If that is
 the case, try to move to a coordinate that lets the pal through. This should
 make monsters using this better at surrounding the player.
 ++===========================================================================*/

class AI_makeRoomForFriend {
public:
  static bool action(Monster* monsterActing, Engine* engine) {
    if(monsterActing->deadState == actorDeadState_alive) {

      bool blockersVision[MAP_X_CELLS][MAP_Y_CELLS];
      engine->mapTests->makeVisionBlockerArray(monsterActing->pos, blockersVision);

      if(monsterActing->checkIfSeeActor(*engine->player, blockersVision)) {
        const unsigned int LOOP_SIZE = engine->gameTime->getLoopSize();
        Monster* otherMonster = NULL;
        Actor* actor = NULL;

        // Loop through all timed entities...
        for(unsigned int i = 0; i < LOOP_SIZE; i++) {
          actor = engine->gameTime->getActorAt(i);

          // If this is a monster other than the current actor...
          if(actor != engine->player && actor != monsterActing) {
            if(actor->deadState == actorDeadState_alive) {
              otherMonster = dynamic_cast<Monster*>(actor);

              bool foundNeighbourWithoutPlayerVis = isNeighbourAndWithoutVision(monsterActing, otherMonster, blockersVision, engine);

              // If the pal sees the player, or it's a neighbour that does not see the player...
              if(otherMonster->checkIfSeeActor(*engine->player, blockersVision) || foundNeighbourWithoutPlayerVis) {

                // If we are indeed blocking a pal, check every neighbouring cell that is at equal
                // or closer distance to the player, to check whether they are fine.

                // TODO Vision must be checked from the cell candidates!

                // TODO If several good candidates are found, result should be picked from them at random

                if(checkIfBlockingMonster(monsterActing->pos, otherMonster, engine) || foundNeighbourWithoutPlayerVis) {

                  // Get a list of neighbouring free cells
                  vector<coord> candidates = getMoveCandidates(monsterActing, engine);

                  // Sort the list by vicinity to player
                  IsCloserToOrigin sorter(engine->player->pos, engine);
                  sort(candidates.begin(), candidates.end(), sorter);

                  // Test the candidate cells until one is found that is not also blocking someone.
                  for(unsigned int ii = 0; ii < candidates.size(); ii++) {

                    bool goodCandidateFound = true;

                    for(unsigned int iii = 0; iii < LOOP_SIZE; iii++) {
                      actor = engine->gameTime->getActorAt(iii);
                      if(actor != engine->player && actor != monsterActing) {
                        otherMonster = dynamic_cast<Monster*>(actor);
                        foundNeighbourWithoutPlayerVis =
                          isNeighbourAndWithoutVision(monsterActing, otherMonster, blockersVision, engine);
                        if(otherMonster->checkIfSeeActor(*engine->player, blockersVision) || foundNeighbourWithoutPlayerVis) {
                          if(checkIfBlockingMonster(candidates.at(ii), otherMonster, engine)
                              || foundNeighbourWithoutPlayerVis) {
                            goodCandidateFound = false;
                          }
                        }
                      }
                    }

                    if(goodCandidateFound) {
                      monsterActing->moveToCell(candidates.at(ii));
                      return true;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    return false;
  }

private:
  static bool checkIfBlockingMonster(const coord& pos, Monster* other, Engine* engine) {
    const vector<coord> line = engine->mapTests->getLine(other->pos.x, other->pos.y, engine->player->pos.x, engine->player->pos.y, true, 99999);
    for(unsigned int i = 0; i < line.size(); i++) {
      if(pos == line.at(i)) {
        return true;
      }
    }
    return false;
  }

  //Returns all the coordinates around the acting monster, that
  //is not occupied by landscape, actors or blocking features.
  static vector<coord> getMoveCandidates(Monster* self, Engine* engine) {
    vector<coord> ret;

    const int PLAYER_X = engine->player->pos.x;
    const int PLAYER_Y = engine->player->pos.y;
    const int OLD_X = self->pos.x;
    const int OLD_Y = self->pos.y;

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    engine->mapTests->makeMoveBlockerArray(self, blockers);

    for(int x = -1; x <= 1; x++) {
      for(int y = -1; y <= 1; y++) {
        if(x != OLD_X || y != OLD_Y) {
          const int NEW_X = OLD_X + x;
          const int NEW_Y = OLD_Y + y;
          const int DIST_CUR = engine->basicUtils->chebyshevDistance(OLD_X, OLD_Y, PLAYER_X, PLAYER_Y);
          const int DIST_NEW = engine->basicUtils->chebyshevDistance(NEW_X, NEW_Y, PLAYER_X, PLAYER_Y);

          if(DIST_NEW <= DIST_CUR) {
            if(blockers[NEW_X][NEW_Y] == false) {
              ret.push_back(coord(NEW_X, NEW_Y));
            }
          }
        }
      }
    }
    return ret;
  }

  /*
   The purpose of the following test is to handle this situation:
   #####
   #.A.#
   #@#B#
   #####
   */
  static bool isNeighbourAndWithoutVision(
    const Monster* const self, Monster* const other,
    bool blockersVision[MAP_X_CELLS][MAP_Y_CELLS], Engine* engine) {
    //If the pal is next to me
    if(engine->mapTests->isCellsNeighbours(self->pos, other->pos, false)) {
      //If pal does not see player
      if(other->checkIfSeeActor(*engine->player, blockersVision) == false) {
        return true;
      }
    }
    return false;
  }
};

#endif
