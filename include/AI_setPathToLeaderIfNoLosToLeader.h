#ifndef AI_SET_PATH_TO_LEADER_IF_NO_LOS_TO_LEADER
#define AI_SET_PATH_TO_LEADER_IF_NO_LOS_TO_LEADER

#include "Engine.h"

class AI_setPathToLeaderIfNoLosToleader {
public:
  static void learn(Monster& monster, vector<Pos>& path, Engine& engine) {

    if(monster.deadState == ActorDeadState::alive) {
      Actor* leader = monster.leader;
      if(leader != NULL) {
        if(leader->deadState == ActorDeadState::alive) {
          bool blockers[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksVision(engine), blockers);

          if(
            engine.fov->checkCell(
              blockers, leader->pos, monster.pos, true)) {
            path.resize(0);
            return;
          }

          MapParse::parse(CellPred::BlocksActor(monster, false, engine),
                          blockers);

          MapParse::parse(CellPred::LivingActorsAdjToPos(monster.pos, engine),
                          blockers, MapParseWriteRule::writeOnlyTrue);

          PathFind::run(monster.pos, leader->pos, blockers, path);
          return;
        }
      }
    }

    path.resize(0);

  }
private:
};



#endif
