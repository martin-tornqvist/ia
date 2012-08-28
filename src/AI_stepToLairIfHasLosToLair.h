#ifndef AI_STEP_TO_LAIR_IF_HAS_LOS_TO_LAIR_H
#define AI_STEP_TO_LAIR_IF_HAS_LOS_TO_LAIR_H

#include "Engine.h"
#include "Fov.h"

class AI_stepToLairIfHasLosToLair {
public:
	static bool action(Monster* monster, const coord& lairCell, Engine* engine) {
		if(monster->deadState == actorDeadState_alive) {
			bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
			engine->mapTests->makeVisionBlockerArray(blockers);
			const bool HAS_LOS_TO_LAIR = engine->fov->checkOneCell(blockers, lairCell.x, lairCell.y, monster->pos.x, monster->pos.y, true);

			if(HAS_LOS_TO_LAIR == true) {
				coord delta = lairCell - monster->pos;

				delta.x = delta.x == 0 ? 0 : (delta.x > 0 ? 1 : -1);
				delta.y = delta.y == 0 ? 0 : (delta.y > 0 ? 1 : -1);
				const coord newPos = monster->pos + delta;

				engine->mapTests->makeMoveBlockerArray(monster, blockers);
				if(blockers[newPos.x][newPos.y]) {
					return false;
				} else {
					monster->moveToCell(monster->pos + delta);
					return true;
				}
			}
		}

		return false;
	}

private:

};

#endif
