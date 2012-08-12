#include "Bash.h"

#include "Engine.h"

#include "GameTime.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"

void Bash::playerBash() const {
	coord bashInPos(eng->player->pos);

	eng->log->addMessage("Bash in what direction? [Space/Esc] Cancel", clrWhiteHigh);
	eng->renderer->flip();
	bashInPos = eng->player->pos + eng->query->direction();
	eng->log->clearLog();
   playerBashFeature(eng->map->featuresStatic[bashInPos.x][bashInPos.y]);
}

void Bash::playerBashFeature(Feature* const feature) const {
	bool bashableObjectFound = false;

	if(feature->getId() == feature_door) {
		Door* const door = dynamic_cast<Door*>(feature);
		door->tryBash(eng->player);
		bashableObjectFound = true;
	}

	if(bashableObjectFound == false) {
		const bool PLAYER_IS_BLIND = eng->player->getStatusEffectsHandler()->allowSee();
		if(PLAYER_IS_BLIND == false) {
			eng->log->addMessage("I see nothing there to bash.");
		} else {
			eng->log->addMessage("I find nothing there to bash.");
		}
	}
}
