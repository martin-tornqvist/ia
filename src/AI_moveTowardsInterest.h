#ifndef AI_MOVE_TOWARDS_INTEREST_H
#define AI_MOVE_TOWARDS_INTEREST_H

#include "Engine.h"
#include "Actor.h"

/*===========================================================================++
Purpose:
Given an array of integers, this class moves the actor in one step towards
the cell with highest "interest"-integer.
Other actors are only considered blocking only when next to source actor.
++===========================================================================*/
/*

class AI_moveTowardsInterest
{
public:
	static bool move(Monster* monster, int arrayOfInterest[MAP_X_CELLS][MAP_Y_CELLS], Engine* engine)
	{
		if(monster->deadState == actorDeadState_alive) {
			//Get most interesting cell
			const coord targetCell(getMostInterestingCell(arrayOfInterest));

			//If no good cell was found, quit
			if(targetCell.x == -1) return false;

			engine->mapTests->makeBlockingArray(actorsBlock_never, true, false, false);

			bool vision[MAP_X_CELLS][MAP_Y_CELLS] = {};
			engine->fov->runFovOnArray(engine->mapTests->blockingArray, monster->pos.x, monster->pos.x, vision);

			for(int x = 0; x < MAP_X_CELLS; x++) {
				for(int y = 0; y < MAP_Y_CELLS; y++) {
					if(vision[x][y] == true) {
						arrayOfInterest[x][y] = 0;
					}
				}
			}

			//Get blocking array
			coord source = monster->pos;
			engine->mapTests->makeBlockingArray(actorsBlock_adjToSource, true, false, false, source);

			//Get path towards cell
			vector<coord> path = engine->pathfinder->findPath(
				monster->pos.x, monster->pos.y, engine->mapTests->blockingArray, targetCell.x, targetCell.y);

			coord result(-1, -1);

			//Get first step in path
			if(path.size() > 0) {
				result = path.back();

				if(result.x != -1) {
					monster->moveToCell(result);
					return true;
				}
			}

		}
		return false;
	}

	static coord getMostInterestingCell(int arrayOfInterest[MAP_X_CELLS][MAP_Y_CELLS])
	{
		coord result(-1, -1);
		int   highestVal = 0;

		for(int x = 0; x < MAP_X_CELLS; x++) {
			for(int y = 0; y < MAP_Y_CELLS; y++) {

				if(arrayOfInterest[x][y] > highestVal) {
					highestVal = arrayOfInterest[x][y];
					result.set(x,y);
				}
			}
		}

		//If result is to the far right, just set it to -1,-1
		//so pathfinding et c won't kick in
		if(result.x == MAP_X_CELLS - 1)
			result.set(-1,-1);

		return result;
	}

private:

};
*/

#endif
