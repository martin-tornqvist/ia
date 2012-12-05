#include "Sound.h"

#include <iostream>

#include "Engine.h"
#include "Feature.h"
#include "Map.h"
#include "ActorPlayer.h"

bool SoundEmitter::isSoundHeardAtRange(const int RANGE, const Sound& sound) const {
	return sound.isLoud() ? RANGE <= 20 : RANGE <= 10;
}

void SoundEmitter::emitSound(Sound sound) const {
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	FeatureStatic* f = NULL;
	for(int y = MAP_Y_CELLS - 1; y >= 0; y--) {
		for(int x = MAP_X_CELLS - 1; x >= 0; x--) {
			f = eng->map->featuresStatic[x][y];
			const bool SOUND_CAN_PASS_CELL = f->isMoveTypePassable(moveType_ooze) == true || f->isBottomless() == true;
			blockers[x][y] = SOUND_CAN_PASS_CELL == false;
		}
	}
	int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeFloodFill(sound.getOrigin(), blockers, floodFill, 999, coord(-1, -1));

	const unsigned int LOOP_SIZE = eng->gameTime->getLoopSize();

	for(unsigned int i = 0; i < LOOP_SIZE; i++) {
		Actor* const actor = eng->gameTime->getActorAt(i);

		const int FLOOD_VALUE_AT_ACTOR = floodFill[actor->pos.x][actor->pos.y];

		if(isSoundHeardAtRange(FLOOD_VALUE_AT_ACTOR, sound)) {
			if(actor == eng->player) {
				//If player, add a direction string to the message (i.e. "(NW)", "(E)" , etc)
				if(sound.getMessage() != "") {
					sound.addString("(" + getPlayerToOriginDirectionString(FLOOD_VALUE_AT_ACTOR, sound.getOrigin(), floodFill) + ")");
				}
			}
			actor->hearSound(sound);
		}
	}
}

string SoundEmitter::getPlayerToOriginDirectionString(const int FLOOD_VALUE_AT_PLAYER,
      const coord& origin, int floodFill[MAP_X_CELLS][MAP_Y_CELLS]) const {

	const coord& playerPos = eng->player->pos;
	string sourceDirectionName = "";

	for(int dx = -1; dx <= 1; dx++) {
		for(int dy = -1; dy <= 1; dy++) {

			const coord checkedCoord = playerPos + coord(dx, dy);

			//If player is next to origin, simply return the direction checked in.
			if(checkedCoord == origin) {
				return directionNames.directions[dx + 1][dy + 1];
			} else {
				//Origin is further away
				const int currentValue = floodFill[checkedCoord.x][checkedCoord.y];
				//If current value is less than players, this is the direction of the sound.
				if(currentValue < FLOOD_VALUE_AT_PLAYER && currentValue != 0) {
					sourceDirectionName = directionNames.directions[dx + 1][dy + 1];
					//If cardinal direction, stop search (To give priority to cardinal directions)
					if(dx == 0 || dy == 0)
						return sourceDirectionName;
				}
			}
		}
	}
	return sourceDirectionName;
}

