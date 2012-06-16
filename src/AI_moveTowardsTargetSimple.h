#ifndef AI_MOVE_TOWARDS_TARGET_SIMPLE
#define AI_MOVE_TOWARDS_TARGET_SIMPLE

#include "Engine.h"
#include "Actor.h"

class AI_moveTowardsTargetSimple {
public:
	static bool action(Monster* monster, Engine* engine) {
		if(monster->deadState == actorDeadState_alive) {
			if(monster->target != NULL) {
				if(monster->playerAwarenessCounter > 0 || monster->leader == engine->player) {
					coord result;
					coord delta = monster->target->pos - monster->pos;
					delta.x = delta.x == 0 ? 0 : (delta.x > 0 ? 1 : -1);
					delta.y = delta.y == 0 ? 0 : (delta.y > 0 ? 1 : -1);
					const coord newPos(monster->pos + delta);
					bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
					engine->mapTests->makeMoveBlockerArray(monster, blockers);
					if(blockers[newPos.x][newPos.y] == false) {
						monster->moveToCell(newPos);
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
