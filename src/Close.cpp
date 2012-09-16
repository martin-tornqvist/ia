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
	coord closeInPos(eng->player->pos);

	eng->log->addMessage("Close in what direction? [Space/Esc] Cancel", clrWhiteHigh);
	eng->renderer->flip();
	closeInPos = eng->player->pos + eng->query->direction();
	eng->log->clearLog();

	if(closeInPos != eng->player->pos) {
		playerCloseFeature(eng->map->featuresStatic[closeInPos.x][closeInPos.y]);
	}
}

void Close::playerCloseFeature(Feature* const feature) const {
	bool closeAbleObjectFound = false;

	if(feature->getId() == feature_door) {
		Door* const door = dynamic_cast<Door*>(feature);
		door->tryClose(eng->player);
		closeAbleObjectFound = true;
	}

	if(closeAbleObjectFound == false) {
		const bool PLAYER_CAN_SEE = eng->player->getStatusEffectsHandler()->allowSee();
		if(PLAYER_CAN_SEE) {
			eng->log->addMessage("I see nothing there to close.");
		} else {
			eng->log->addMessage("I find nothing there to close.");
		}
	}
}

