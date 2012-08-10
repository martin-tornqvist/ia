#include "JamWithSpike.h"

#include "Engine.h"
#include "GameTime.h"
#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Query.h"
#include "Inventory.h"

void JamWithSpike::playerJam() const {
	if(eng->player->getInventory()->hasItemInGeneral(item_ironSpike) == false) {
		eng->log->addMessage("You have no spikes to jam with.", clrWhite);
		return;
	}

	coord jamInPos(eng->player->pos);

	eng->log->addMessage("Jam with spike in what direction? [Space/Esc] Cancel", clrWhiteHigh);
	eng->renderer->flip();
	jamInPos = eng->player->pos + eng->query->direction();
	eng->log->clearLog();

   playerJamFeature(eng->map->featuresStatic[jamInPos.x][jamInPos.y]);
}

void JamWithSpike::playerJamFeature(Feature* const feature) const {
	bool jamableObjectFound = false;

	if(feature->getId() == feature_door) {
		Door* const door = dynamic_cast<Door*>(feature);
		const bool DOOR_SPIKED = door->trySpike(eng->player);

		if(DOOR_SPIKED == true) {

			jamableObjectFound = true;

			eng->player->getInventory()->decreaseItemTypeInGeneral(item_ironSpike);
			const int SPIKES_LEFT = eng->player->getInventory()->getItemStackSizeInGeneral(item_ironSpike);
			if(SPIKES_LEFT == 0) {
				eng->log->addMessage("You have no iron spikes left.");
			} else {
				eng->log->addMessage("You have " + intToString(SPIKES_LEFT) + " iron spikes left.");
			}
		}
	}

	if(jamableObjectFound == false) {
		const bool PLAYER_IS_BLIND = eng->player->getStatusEffectsHandler()->allowSee();
		if(PLAYER_IS_BLIND == false) {
			eng->log->addMessage("You see nothing there to jam with a spike.");
		} else {
			eng->log->addMessage("You find nothing there to jam with a spike.");
		}
	}
}

