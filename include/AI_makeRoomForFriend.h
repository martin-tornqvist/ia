#ifndef AI_MAKE_ROOM_FOR_FRIEND_H
#define AI_MAKE_ROOM_FOR_FRIEND_H

#include <algorithm>

#include "MapParsing.h"
#include "LineCalc.h"

class AI_makeRoomForFriend {
public:
  static bool action(Monster& monster, Engine* engine) {
    if(monster.deadState == actorDeadState_alive) {

      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      MapParser::parse(CellPredBlocksVision(engine), visionBlockers);

      if(monster.checkIfSeeActor(*engine->player, visionBlockers)) {
        const int NR_ACTORS = engine->gameTime->getNrActors();
        Monster* otherMonster = NULL;
        Actor* actor = NULL;

        //Loop through all actors
        for(int i = 0; i < NR_ACTORS; i++) {
          actor = &(engine->gameTime->getActorAtElement(i));
          if(actor != engine->player && actor != &monster) {
            if(actor->deadState == actorDeadState_alive) {
              otherMonster = dynamic_cast<Monster*>(actor);

              bool isOtherAdjWithoutVision =
                isAdjAndWithoutVision(
                  monster, *otherMonster, visionBlockers, engine);

              //Other monster sees the player, or it's a neighbour that
              //does not see the player?
              if(
                otherMonster->checkIfSeeActor(
                  *engine->player, visionBlockers) ||
                isOtherAdjWithoutVision) {

                // If we are indeed blocking a pal, check every neighbouring
                //cell that is at equal or closer distance to the player,
                //to check whether they are fine.

                // TODO Vision must be checked from the cell candidates!

                // TODO If several good candidates are found,
                //result should be picked from them at random

                if(
                  checkIfBlockingMonster(
                    monster.pos, otherMonster, engine) ||
                  isOtherAdjWithoutVision) {

                  // Get a list of neighbouring free cells
                  vector<Pos> candidates;
                  getMoveCandidates(monster, candidates, engine);

                  // Sort the list by closeness to player
                  IsCloserToOrigin sorter(engine->player->pos, engine);
                  sort(candidates.begin(), candidates.end(), sorter);

                  // Test the candidate cells until one is found that
                  //is not also blocking someone.
                  const int NR_CANDIDATES = candidates.size();
                  for(int ii = 0; ii < NR_CANDIDATES; ii++) {

                    bool isGoodCandidateFound = true;

                    for(int iii = 0; iii < NR_ACTORS; iii++) {
                      actor = &(engine->gameTime->getActorAtElement(iii));
                      if(actor != engine->player && actor != &monster) {
                        otherMonster = dynamic_cast<Monster*>(actor);
                        if(
                          otherMonster->checkIfSeeActor(
                            *engine->player, visionBlockers)) {
                          if(
                            checkIfBlockingMonster(
                              candidates.at(ii), otherMonster, engine)) {
                            isGoodCandidateFound = false;
                            break;
                          }
                        }
                      }
                    }

                    if(isGoodCandidateFound) {
                      const Pos offset = candidates.at(ii) - monster.pos;
                      monster.moveDir(DirConverter(engine).getDir(offset));
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
  static bool checkIfBlockingMonster(const Pos& pos, Monster* other,
                                     Engine* engine) {

    vector<Pos> line;
    engine->lineCalc->calcNewLine(
      other->pos, engine->player->pos, true, 9999, false, line);
    for(unsigned int i = 0; i < line.size(); i++) {
      if(line.at(i) == pos) {
        return true;
      } else if(line.at(i) == engine->player->pos) {
        return false;
      }
    }
    return false;
  }

  //Returns all free positions around the acting monster that is further
  //from the player than the monster's current position
  static void getMoveCandidates(Monster& self, vector<Pos>& dirsToMake,
                                Engine* eng) {

    dirsToMake.resize(0);

    const int PLAYER_X = eng->player->pos.x;
    const int PLAYER_Y = eng->player->pos.y;
    const int OLD_X = self.pos.x;
    const int OLD_Y = self.pos.y;

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(CellPredBlocksBodyType(self.getBodyType(), eng), blockers);

    for(int x = -1; x <= 1; x++) {
      for(int y = -1; y <= 1; y++) {
        if(x != OLD_X || y != OLD_Y) {
          const int NEW_X = OLD_X + x;
          const int NEW_Y = OLD_Y + y;
          const int DIST_CUR =
            eng->basicUtils->chebyshevDist(OLD_X, OLD_Y, PLAYER_X, PLAYER_Y);
          const int DIST_NEW =
            eng->basicUtils->chebyshevDist(NEW_X, NEW_Y, PLAYER_X, PLAYER_Y);

          if(DIST_NEW <= DIST_CUR) {
            if(blockers[NEW_X][NEW_Y] == false) {
              dirsToMake.push_back(Pos(NEW_X, NEW_Y));
            }
          }
        }
      }
    }
  }

  /*
   The purpose of the following test is to handle this situation:
   #####
   #.A.#
   #@#B#
   #####
   */
  static bool isAdjAndWithoutVision(
    const Monster& self, Monster& other,
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS], Engine* engine) {

    //If the pal is next to me
    if(engine->basicUtils->isPosAdj(self.pos, other.pos, false)) {
      //If pal does not see player
      if(other.checkIfSeeActor(*engine->player, visionBlockers) == false) {
        return true;
      }
    }
    return false;
  }
};

#endif
