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
		eng->log->addMessage("You have no spikes to jam anything with.", clrWhite);
	} else {

		//Store the coordinates of all adjacent explored doors (if explicit direction is not used).
		vector<coord> adjDoorPositions;
		if(eng->config->ALWAYS_ASK_DIRECTION == 0) {
			for(int dx = -1; dx <= 1; dx++) {
				for(int dy = -1; dy <= 1; dy++) {
					if(dx != 0 || dy != 0) {
						const coord c(eng->player->pos + coord(dx, dy));
						Feature* const f = eng->map->featuresStatic[c.x][c.y];
						if(f->getId() == feature_door) {
							Door* const door = dynamic_cast<Door*> (f);
							if(door->isSecret() == false && eng->map->explored[c.x][c.y] == true) {
								adjDoorPositions.push_back(c);
							}
						}
					}
				}
			}
		}

		coord jamInPos; //Position of object to jam.

		//If there is exactly one adjacent door (and explicit direction is not used), use that position.
		if(adjDoorPositions.size() == 1 && eng->config->ALWAYS_ASK_DIRECTION == 0) {
			jamInPos = adjDoorPositions.at(0);
		} else {
			//Else, ask for direction.
			eng->log->addMessage("Jam with spike in what direction? [Space/Esc] Cancel", clrWhiteHigh);
			eng->renderer->flip();
			jamInPos = eng->player->pos + eng->query->direction();
			eng->log->clearLog();
		}

		if(jamInPos != eng->player->pos) {
			bool jamableObjectFound = false;

			Feature* const f = eng->map->featuresStatic[jamInPos.x][jamInPos.y];
			if(f->getId() == feature_door) {
				Door* const door = dynamic_cast<Door*> (f);
				const bool DOOR_SPIKED = door->trySpike(eng->player);
				jamableObjectFound = DOOR_SPIKED;

				if(DOOR_SPIKED == true) {
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
	}
}

