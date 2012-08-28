#include "DungeonClimb.h"

#include "Engine.h"
#include "SpawnCounter.h"
#include "Map.h"
#include "MapBuildBSP.h"
#include "MapBuild.h"
#include "Populate.h"
#include "MapBuildSpawnItems.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Item.h"

#include "PlayerAllocBonus.h"

extern void profiler(bool init);

void DungeonClimb::makeLevel() {
	const int DLVL = eng->map->getDungeonLevel();
	bool levelBuilt = false;
	if(eng->config->BOT_PLAYING == false) {
		//------------------------------------- TRAPEZOHEDRON LEVEL
		if(levelBuilt == false) {
			if(DLVL > LAST_CAVERN_LEVEL) {
				eng->mapBuild->buildTrapezohedronLevel();
				levelBuilt = true;
			}
		}

		//------------------------------------- KINGS TOMB
		if(levelBuilt == false) {
			if(DLVL == FIRST_CAVERN_LEVEL - 1) {
				eng->mapBuild->buildKingsTomb();
				levelBuilt = true;
			}
		}
	}
	//------------------------------------- DUNGEON LEVELS
	if(levelBuilt == false) {
		if(DLVL < FIRST_CAVERN_LEVEL || eng->config->BOT_PLAYING) {
			//eng->mapBuild->buildDungeonLevel();
			eng->mapBuildBSP->run();
			eng->populate->populate();
			levelBuilt = true;
		}
	}
	//------------------------------------- CAVERN LEVELS
	if(levelBuilt == false) {
		if(DLVL >= FIRST_CAVERN_LEVEL) {
			eng->mapBuild->buildCavern();
			eng->populate->populate();
		}
	}
	if(DLVL > 0 && DLVL <= LAST_CAVERN_LEVEL) {
		eng->mapBuildSpawnItems->spawnItems();
	}
}

void DungeonClimb::travelDown(const int levels) {
	eng->player->insanityShort = 0;
	eng->player->insanityShortTemp = 0;

	eng->map->incrDungeonLevel(levels);
	makeLevel();

	eng->player->target = NULL;
	eng->player->FOVupdate();
	eng->player->updateColor();
	eng->renderer->drawMapAndInterface();
}

void DungeonClimb::attemptUseDownStairs() {
	const int DLVL = eng->map->getDungeonLevel();

	const coord& playerPos = eng->player->pos;
	if(eng->map->featuresStatic[playerPos.x][playerPos.y]->getId() == feature_stairsDown) {
		if(DLVL >= FIRST_CAVERN_LEVEL && DLVL <= LAST_CAVERN_LEVEL) {
			eng->log->addMessage("I climb downwards.");
		} else {
			eng->log->addMessage("I descend the stairs.");
		}
		eng->renderer->flip();
		travelDown();
	} else {
		if(DLVL >= FIRST_CAVERN_LEVEL && DLVL <= LAST_CAVERN_LEVEL) {
			eng->log->addMessage("I see no path leading downwards here.");
		} else {
			eng->log->addMessage("I see no stairs leading downwards here.");
		}
		eng->renderer->flip();
	}
}

