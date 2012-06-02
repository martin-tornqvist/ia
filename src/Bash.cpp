#include "Bash.h"

#include "Engine.h"

#include "GameTime.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"

void Bash::playerBash() const {
	//Store the coords of all adjacent explored doors (if explicit direction is not used).
	vector<coord> adjDoorPositions;
	if(eng->config->ALWAYS_ASK_DIRECTION == 0) {
		for(int dx = -1; dx <= 1; dx++) {
			for(int dy = -1; dy <= 1; dy++) {
				if(dx != 0 || dy != 0) {
					const coord c(eng->player->pos + coord(dx, dy));
					Feature* const f = eng->map->featuresStatic[c.x][c.y];
					if(f->getId() == feature_door) {
						Door* const door = dynamic_cast<Door*> (f);
						if(door->isSecret() == false && eng->map->explored[c.x][c.y]) {
							adjDoorPositions.push_back(c);
						}
					}
				}
			}
		}
	}

	coord bashInPos; //Position of object to bash.

	//If there is exactly one adjacent door (and explicit direction is not used), use that position.
	if(adjDoorPositions.size() == 1 && eng->config->ALWAYS_ASK_DIRECTION == 0) {
		bashInPos = adjDoorPositions.at(0);
	} else {
		//Else, ask for direction.
		eng->log->addMessage("Bash in what direction? [Space/Esc] Cancel", clrWhiteHigh);
		eng->renderer->flip();
		bashInPos = eng->player->pos + eng->query->direction();
		eng->log->clearLog();
	}

	if(bashInPos != eng->player->pos) {
		bool bashableObjectFound = false;

		Feature* const f = eng->map->featuresStatic[bashInPos.x][bashInPos.y];
		if(f->getId() == feature_door) {
			Door* const door = dynamic_cast<Door*> (f);
			bashableObjectFound = true;
			door->tryBash(eng->player);
		}

		if(bashableObjectFound == false) {
			const bool PLAYER_IS_BLIND = eng->player->getStatusEffectsHandler()->allowSee();
			if(PLAYER_IS_BLIND == false) {
				eng->log->addMessage("You see nothing there to bash.");
			} else {
				eng->log->addMessage("You find nothing there to bash.");
			}
		}
	}
}

//void Bash::monsterBashDirection(Actor* const actorBashing, const coord dir) const
//{
//	//Get bash position
//	const coord origin   = actorBashing->pos;
//	const coord pos      = origin + dir;
//
//	//Check for bashable object there.
//	bool bashableObjectFound = false;
//
//	Door* door = eng->mapdoors[pos.x][pos.y];
//	if(door != NULL) {
//		bashableObjectFound = true;
//		door->tryBash(actorBashing);
//	}
//
//	if(bashableObjectFound == false) {
//		cout << "[WARNING] AI tried to bash cell without bashable object" << endl;
//	}
//}


