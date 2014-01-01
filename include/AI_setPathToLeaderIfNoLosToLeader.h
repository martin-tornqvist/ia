#ifndef AI_SET_PATH_TO_LEADER_IF_NO_LOS_TO_LEADER
#define AI_SET_PATH_TO_LEADER_IF_NO_LOS_TO_LEADER

#include "Engine.h"

class AI_setPathToLeaderIfNoLosToleader {
public:
  static void learn(Monster& monster, vector<Pos>& path, Engine& engine) {

    if(monster.deadState == actorDeadState_alive) {
      Actor* leader = monster.leader;
      if(leader != NULL) {
        if(leader->deadState == actorDeadState_alive) {
          bool blockers[MAP_W][MAP_H];
          MapParser::parse(CellPredBlocksVision(engine), blockers);

          if(
            engine.fov->checkCell(
              blockers, leader->pos, monster.pos, true)) {
            path.resize(0);
            return;
          }

          MapParser::parse(
            CellPredBlocksBodyType(monster.getBodyType(), false, engine),
            blockers);
          MapParser::parse(CellPredLivingActorsAdjToPos(monster.pos, engine),
                           blockers, mapParseWriteOnlyTrue);
          engine.pathFinder->run(
            monster.pos, leader->pos, blockers, path);
          return;
        }
      }
    }

    path.resize(0);

  }
private:
};



#endif
