#include "MapBuild.h"

#include "Engine.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "ItemFactory.h"

void MapBuild::buildTrapezohedronLevel() {
	eng->map->clearDungeon();

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			eng->featureFactory->spawnFeatureAt(feature_caveWall, coord(x, y));
		}
	}

	eng->mapBuild->makePathByRandomWalk(eng->player->pos.x, eng->player->pos.y, 150, feature_caveFloor, true);
	eng->mapBuild->makePathByRandomWalk(MAP_X_CELLS_HALF, MAP_Y_CELLS_HALF, 800, feature_caveFloor, true);
	eng->mapBuild->makeStraightPathByPathfinder(eng->player->pos, coord(MAP_X_CELLS_HALF, MAP_Y_CELLS_HALF), feature_caveFloor, false, true);

	eng->itemFactory->spawnItemOnMap(item_trapezohedron, coord(MAP_X_CELLS_HALF, MAP_Y_CELLS_HALF));

}
