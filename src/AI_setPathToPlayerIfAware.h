#ifndef AI_SET_PATH_TO_PLAYER_IF_AWARE_H
#define AI_SET_PATH_TO_PLAYER_IF_AWARE_H

#include "Engine.h"
#include "Pathfinding.h"

class AI_setPathToPlayerIfAware {
public:
	static void learn(Monster* monster, vector<coord>* path, Engine* engine) {
		if(monster->deadState == actorDeadState_alive) {
			if(monster->playerAwarenessCounter > 0) {
				//TODO "virtual bool isMovePassableForPathfindingMonster()" in features to avoid the stuff below.
				const bool CAN_OPEN_DOORS = monster->getInstanceDefinition()->canOpenDoors;
				const bool CAN_BASH_DOORS = monster->getInstanceDefinition()->canBashDoors;
				const bool CONSIDER_DOORS_FREE = monster->playerAwarenessCounter > 0 && (CAN_OPEN_DOORS || CAN_BASH_DOORS);
				bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
				engine->basicUtils->resetBoolArray(blockers, false);
				for(int y = 0; y < MAP_Y_CELLS; y++) {
					for(int x = 0; x < MAP_X_CELLS; x++) {
						if(engine->map->featuresStatic[x][y]->isMovePassable(monster) == false) {
							if(engine->map->featuresStatic[x][y]->getId() == feature_door) {
								if(CONSIDER_DOORS_FREE == false) {
									blockers[x][y] = true;
								}
							} else {
								blockers[x][y] = true;
							}
						}
					}
				}
				FeatureMob* f = NULL;
				const unsigned int FEATURE_MOBS_SIZE = engine->gameTime->getFeatureMobsSize();
				for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
					f = engine->gameTime->getFeatureMobAt(i);
					if(blockers[f->getX()][f->getY()] == false) {
						blockers[f->getX()][f->getY()] = !f->isShootPassable();
					}
				}
				engine->mapTests->addAdjacentLivingActorsToBlockerArray(monster->pos, blockers);

				*path = engine->pathfinder->findPath(monster->pos, blockers, engine->player->pos);
			} else {
				path->resize(0);
			}
		} else {
			path->resize(0);
		}
	}

private:
};

#endif

