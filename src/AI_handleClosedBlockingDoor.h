#ifndef AI_OPEN_DOOR
#define AI_OPEN_DOOR

#include "Map.h"
#include "FeatureDoor.h"

/*
 * Purpose:
 * If first step in path is ordinary closed door, try to open it if able.
 * Otherwise try to bash it if able.
 *
 * Assumptions:
 * The parameter path goes only through doors that can be handled in some
 * way by the monster (or no doors at all).
 */

class AI_handleClosedBlockingDoor {
public:
	static bool action(Monster* monster, vector<coord>* path, Engine* engine) {
		if(monster->deadState == actorDeadState_alive && path->size() != 0) {
			Feature* const f = engine->map->featuresStatic[path->back().x][path->back().y];
			if(f->getId() == feature_door) {
				Door* const door = dynamic_cast<Door*>(f);
				if(door->isMovePassable(monster) == false) {
					if(door->isStuck() == false) {
						if(monster->getInstanceDefinition()->canOpenDoors == true) {
							door->tryOpen(monster);
							return true;
						} else if(monster->getInstanceDefinition()->canBashDoors == true) {
							door->tryBash(monster);
							return true;
						}
					} else if(monster->getInstanceDefinition()->canBashDoors == true) {
						door->tryBash(monster);
						return true;
					}
				}
			}
		}
		return false;
	}

private:
};

#endif
