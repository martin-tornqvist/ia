#include "Sound.h"

#include <iostream>

#include "Engine.h"
#include "Feature.h"
#include "Map.h"
#include "ActorPlayer.h"

bool SoundEmitter::isSoundHeardAtRange(const int range, const int soundStrengthRating) const {
	switch (soundStrengthRating) {
	case 1:
		return range <= 1;
	break; //whispering, soft material, lockpicking, disarming traps...
	case 2:
		return range <= 6;
	break; //opening and closing doors...
	case 3:
		return range <= 9;
	break; //throwing heavy objects, bashing doors, yelling...
	case 4:
		return range <= 12;
	break; //gunfire...
	case 5:
		return range <= 16;
	break; //explosions...

	default: {
		if(soundStrengthRating < 0) {
			cout << "[WARNING] sound strength rating < 0, in isSoundHeardAtRange()" << endl;
			return false;
		}
		if(soundStrengthRating > 5) {
			cout << "[WARNING] sound strength rating > 5, in isSoundHeardAtRange()" << endl;
			return true;
		}
	}
	break;
	}

	return false;
}

void SoundEmitter::emitSound(Sound sound) const {
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	FeatureStatic* f = NULL;
	for(int y = MAP_Y_CELLS - 1; y >= 0; y--) {
		for(int x = MAP_X_CELLS - 1; x >= 0; x--) {
			f = eng->map->featuresStatic[x][y];
			blockers[x][y] = !f->isMoveTypePassable(moveType_ooze) && !f->isBottomless();
		}
	}
	int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeFloodFill(sound.getOrigin(), blockers, floodFill, 999, coord(-1, -1));

	const unsigned int LOOP_SIZE = eng->gameTime->getLoopSize();

	for(unsigned int i = 0; i < LOOP_SIZE; i++) {
		Actor* const actor = eng->gameTime->getActorAt(i);

		const int FLOOD_VALUE_AT_ACTOR = floodFill[actor->pos.x][actor->pos.y];

		if(isSoundHeardAtRange(FLOOD_VALUE_AT_ACTOR, sound.getStrength())) {
			if(actor == eng->player) {
				//If player, add a direction string to the message (i.e. "(NW)", "(E)" , etc)
				if(sound.getMessage() != "") {
					string directionMessage = getPlayerToOriginDirectionString(FLOOD_VALUE_AT_ACTOR, sound.getOrigin(), floodFill);
					directionMessage = "(" + directionMessage + ")";
					sound.addDirectionString(directionMessage);
				}
			}
			actor->registerHeardSound(sound);
		}
	}
}

string SoundEmitter::getPlayerToOriginDirectionString(const int floodValueAtPlayer, const coord origin, int floodFill[MAP_X_CELLS][MAP_Y_CELLS]) const {
	const int playerX = eng->player->pos.x;
	const int playerY = eng->player->pos.y;

	string sourceDirectionName = "";

	for(int i = -1; i <= 1; i++) {
		for(int j = -1; j <= 1; j++) {
			//If player is next to origin, simply return the direction checked in.
			if(playerX + i == origin.x && playerY + j == origin.y) {
				return directionNames.directions[i + 1][j + 1];
			} else {
				//Origin is further away
				const int currentValue = floodFill[playerX + i][playerY + j];
				//If current value is less than players, this is the direction of the sound.
				if(currentValue < floodValueAtPlayer && currentValue != 0) {
					sourceDirectionName = directionNames.directions[i + 1][j + 1];
					//If cardinal direction, stop search (To give priority to cardinal directions)
					if(i == 0 || j == 0)
						return sourceDirectionName;
				}
			}
		}
	}
	return sourceDirectionName;
}

