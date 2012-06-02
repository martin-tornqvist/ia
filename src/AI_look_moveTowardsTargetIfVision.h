#ifndef AI_MOVE_TOWARDS_TARGET_IF_VISION
#define AI_MOVE_TOWARDS_TARGET_IF_VISION

#include "Engine.h"
#include "Actor.h"

/*=======================================================++
 Purpose:
 Check if monster sees target, if so move towards target,
 if not blocked.

 (If landscape that can be seen through but not moved
 through (i.e. bottomless pits, deep water) is introduced,
 this function will have to use the pathfinder in those
 situations (can see, yet blocked by landscape/features))
 ++=======================================================*/

class AI_look_moveTowardsTargetIfVision {
public:

	static bool action(Monster* monster, Engine* engine) {
		if(monster->deadState == actorDeadState_alive) {
			if(monster->playerAwarenessCounter > 0) {
				coord result;
				Actor* target = engine->player;
				if(target == NULL) {
					return false;
				} else {
					bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
					engine->mapTests->makeVisionBlockerArray(blockers);
					const bool IS_TARGET_SEEN = monster->checkIfSeeActor(*engine->player, blockers);
					if(IS_TARGET_SEEN) {
						coord delta = target->pos - monster->pos;
						delta.x = delta.x == 0 ? 0 : (delta.x > 0 ? 1 : -1);
						delta.y = delta.y == 0 ? 0 : (delta.y > 0 ? 1 : -1);
						const coord newPos(monster->pos + delta);
						engine->mapTests->makeMoveBlockerArray(monster, blockers);
						if(blockers[newPos.x][newPos.y] == false) {
                            monster->moveToCell(newPos);
							return true;
						} else {
                            return false;
						}
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
