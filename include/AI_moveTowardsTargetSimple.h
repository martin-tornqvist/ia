#ifndef AI_MOVE_TOWARDS_TARGET_SIMPLE
#define AI_MOVE_TOWARDS_TARGET_SIMPLE

#include "Engine.h"
#include "Actor.h"
#include "MapParsing.h"

class AI_moveTowardsTargetSimple {
public:
  static bool action(Monster& monster, Engine& engine) {
    if(monster.deadState == ActorDeadState::alive) {
      if(monster.target != NULL) {
        if(
          monster.awareOfPlayerCounter_ > 0 ||
          monster.leader == engine.player) {
          Pos result;
          Pos offset = monster.target->pos - monster.pos;
          offset.x = offset.x == 0 ? 0 : (offset.x > 0 ? 1 : -1);
          offset.y = offset.y == 0 ? 0 : (offset.y > 0 ? 1 : -1);
          bool blockers[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksActor(monster, true, engine),
                          blockers);
          const Pos newPos(monster.pos + offset);
          if(blockers[newPos.x][newPos.y] == false) {
            monster.moveDir(DirConverter().getDir(offset));
            return true;
          } else {
            return false;
          }
        }
      }
    }
    return false;
  }
private:
};

#endif
