#ifndef AI_MAKE_ROOM_FOR_FRIEND_H
#define AI_MAKE_ROOM_FOR_FRIEND_H

#include <algorithm>

#include "MapParsing.h"
#include "LineCalc.h"
#include "Utils.h"

class AI_makeRoomForFriend {
public:
  static bool action(Monster& monster, Engine& engine) {
    if(monster.deadState == actorDeadState_alive) {

      bool visionBlockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksVision(engine), visionBlockers);

      if(monster.isSeeingActor(*engine.player, visionBlockers)) {

        //Loop through all actors
        for(Actor * actor : engine.gameTime->actors_) {
          if(actor != engine.player && actor != &monster) {
            if(actor->deadState == actorDeadState_alive) {
              Monster* otherMonster = dynamic_cast<Monster*>(actor);

              bool isOtherAdjWithoutVision =
                isAdjAndWithoutVision(
                  monster, *otherMonster, visionBlockers, engine);

              //Other monster sees the player, or it's a neighbour that
              //does not see the player?
              if(
                otherMonster->isSeeingActor(
                  *engine.player, visionBlockers) ||
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
                  IsCloserToOrigin sorter(engine.player->pos, engine);
                  sort(candidates.begin(), candidates.end(), sorter);

                  // Test the candidate cells until one is found that
                  //is not also blocking someone.
                  const int NR_CANDIDATES = candidates.size();
                  for(int ii = 0; ii < NR_CANDIDATES; ii++) {

                    bool isGoodCandidateFound = true;

                    for(Actor * actor2 : engine.gameTime->actors_) {
                      if(actor2 != engine.player && actor2 != &monster) {
                        otherMonster = dynamic_cast<Monster*>(actor2);
                        if(
                          otherMonster->isSeeingActor(
                            *engine.player, visionBlockers)) {
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
                      monster.moveDir(DirConverter().getDir(offset));
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
  //Check if acting monster is on a line between player and other monster
  static bool checkIfBlockingMonster(const Pos& pos, Monster* other,
                                     Engine& engine) {
    vector<Pos> line;
    engine.lineCalc->calcNewLine(
      other->pos, engine.player->pos, true, 9999, false, line);

    for(const Pos & posInLine : line) {if(posInLine == pos) {return true;}}
    return false;
  }

  //Returns all free positions around the acting monster that is further
  //from the player than the monster's current position
  static void getMoveCandidates(Monster& self, vector<Pos>& dirsToMake,
                                Engine& eng) {

    dirsToMake.resize(0);

    const int PLAYER_X = eng.player->pos.x;
    const int PLAYER_Y = eng.player->pos.y;
    const int OLD_X = self.pos.x;
    const int OLD_Y = self.pos.y;

    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksActor(self, true, eng), blockers);

    for(int x = -1; x <= 1; x++) {
      for(int y = -1; y <= 1; y++) {
        if(x != OLD_X || y != OLD_Y) {
          const int NEW_X = OLD_X + x;
          const int NEW_Y = OLD_Y + y;
          const int DIST_CUR =
            Utils::chebyshevDist(OLD_X, OLD_Y, PLAYER_X, PLAYER_Y);
          const int DIST_NEW =
            Utils::chebyshevDist(NEW_X, NEW_Y, PLAYER_X, PLAYER_Y);

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
    bool visionBlockers[MAP_W][MAP_H], Engine& engine) {

    //If the pal is next to me
    if(Utils::isPosAdj(self.pos, other.pos, false)) {
      //If pal does not see player
      if(other.isSeeingActor(*engine.player, visionBlockers) == false) {
        return true;
      }
    }
    return false;
  }
};

#endif
