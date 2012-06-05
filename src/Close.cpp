#include "Close.h"

#include "Engine.h"
#include "GameTime.h"
#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "FeatureDoor.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"

void Close::playerClose() const {
	//Store the coords of all adjacent explored doors (if explicit direction is not used).
	vector<coord> adjDoorPositions;
	if(eng->config->ALWAYS_ASK_DIRECTION == 0) {
		for(int dx = -1; dx <= 1; dx++) {
			for(int dy = -1; dy <= 1; dy++) {
				if(dx != 0 || dy != 0) {
					const coord c(eng->player->pos + coord(dx, dy));
					Feature* const f = eng->map->featuresStatic[c.x][c.y];
					if(f->getId() == feature_door) {
						Door* const door = dynamic_cast<Door*>(f);
						if(door->isSecret() == false && eng->map->explored[c.x][c.y] == true) {
							adjDoorPositions.push_back(c);
						}
					}
				}
			}
		}
	}

	coord closeInPos; //Position of object to close.

	//If there is exactly one closable adjacent door (and explicit direction is not used), use that position.
	if(adjDoorPositions.size() == 1 && eng->config->ALWAYS_ASK_DIRECTION == 0) {
		closeInPos = adjDoorPositions.at(0);
	} else {
		//Else, ask for direction.
		eng->log->addMessage("Close in what direction? [Space/Esc] Cancel", clrWhiteHigh);
		eng->renderer->flip();
		closeInPos = eng->player->pos + eng->query->direction();
		eng->log->clearLog();
	}

	if(closeInPos != eng->player->pos) {
		//Check for closable object there.
		bool closeAbleObjectFound = false;

		Feature* const f = eng->map->featuresStatic[closeInPos.x][closeInPos.y];
		if(f->getId() == feature_door) {
			Door* const door = dynamic_cast<Door*>(f);
			closeAbleObjectFound = true;
			door->tryClose(eng->player);
		}

		if(closeAbleObjectFound == false) {
			const bool PLAYER_IS_BLIND = eng->player->getStatusEffectsHandler()->allowSee();
			if(PLAYER_IS_BLIND == false) {
				eng->log->addMessage("You see nothing there to close.");
			} else {
				eng->log->addMessage("You find nothing there to close.");
			}
		}
	}
}

