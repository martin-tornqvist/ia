#ifndef AI_SET_PATH_TO_LEADER_IF_NO_LOS_TO_LEADER
#define AI_SET_PATH_TO_LEADER_IF_NO_LOS_TO_LEADER

#include "Engine.h"

class AI_setPathToLeaderIfNoLosToleader {
public:
	static void learn(Monster* monster, vector<coord>* path, Engine* engine) {

		if(monster->deadState == actorDeadState_alive) {
		   Actor* leader = monster->leader;
         if(leader != NULL) {
            if(leader->deadState == actorDeadState_alive) {
               bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
               engine->mapTests->makeVisionBlockerArray(blockers);

               if(engine->fov->checkOneCell(blockers, leader->pos, monster->pos, true)) {
                  path->resize(0);
                  return;
               }

               engine->mapTests->makeMoveBlockerArrayFeaturesOnly(monster, blockers);
               engine->mapTests->addAdjacentLivingActorsToBlockerArray(monster->pos, blockers);
               *path = engine->pathfinder->findPath(monster->pos, blockers, leader->pos);
               return;
            }
         }
		}

		path->resize(0);

	}
private:
};



#endif
