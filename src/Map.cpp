#include "Map.h"

#include <cassert>

#include "Engine.h"
#include "Feature.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ItemFactory.h"
#include "MapBuildBSP.h"

using namespace std;

Map::Map(Engine* engine) :
  eng(engine), dungeonLevel_(0) {

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      featuresStatic[x][y] = new FeatureStatic(feature_stoneWall, coord(x, y), eng);
      assert(featuresStatic[x][y]->getId() == feature_stoneWall);
    }
  }

  clearGrids(false);
}

Map::~Map() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      delete featuresStatic[x][y];
    }
  }
}

//TODO This should probably go in a virtual method in Feature instead
void Map::switchToDestroyedFeatAt(const coord pos) {
  if(eng->mapTests->isCellInsideMainScreen(pos)) {

    const Feature_t OLD_FEATURE_ID = eng->map->featuresStatic[pos.x][pos.y]->getId();

    const vector<Feature_t> convertionCandidates = eng->featureData->getFeatureDef(OLD_FEATURE_ID)->featuresOnDestroyed;

    const int SIZE = convertionCandidates.size();
    if(SIZE > 0) {
      const Feature_t NEW_FEATURE_ID = convertionCandidates.at(eng->dice(1, SIZE) - 1);

      eng->featureFactory->spawnFeatureAt(NEW_FEATURE_ID, pos);

      //Destroy adjacent doors?
      if((NEW_FEATURE_ID == feature_rubbleHigh || NEW_FEATURE_ID == feature_rubbleLow) && NEW_FEATURE_ID != OLD_FEATURE_ID) {
        for(int x = pos.x - 1; x <= pos.x + 1; x++) {
          for(int y = pos.y - 1; y <= pos.y + 1; y++) {
            if(x == 0 || y == 0) {
              if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
                eng->featureFactory->spawnFeatureAt(feature_rubbleLow, coord(x, y));
              }
            }
          }
        }
      }

      if(NEW_FEATURE_ID == feature_rubbleLow && NEW_FEATURE_ID != OLD_FEATURE_ID) {
        if(eng->dice.percentile() < 50) {
          eng->itemFactory->spawnItemOnMap(item_rock, pos);
        }
      }
    }
  }
}

void Map::clearDungeon() {
  eng->actorFactory->deleteAllMonsters();

  eng->mapBuildBSP->clearRooms();

  clearGrids(true);
  eng->gameTime->eraseAllFeatureMobs();
  eng->gameTime->resetTurnTypeAndActorCounters();
}

void Map::clearGrids(const bool DELETE_INSTANCES) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      explored[x][y] = false;
      playerVision[x][y] = false;

      if(DELETE_INSTANCES == true) {
        eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));

        if(items[x][y] != NULL) {
          delete(items[x][y]);
        }
      }

      items[x][y] = NULL;
      darkness[x][y] = false;
      light[x][y] = false;
      playerVisualMemory[x][y].glyph = ' ';
      playerVisualMemory[x][y].color = clrBlack;
      playerVisualMemoryTiles[x][y].tile = tile_empty;
      playerVisualMemoryTiles[x][y].color = clrBlack;
      eng->renderer->renderArray[x][y].clear();
      eng->renderer->renderArrayActorsOmitted[x][y].clear();
      eng->renderer->renderArrayTiles[x][y].clear();
      eng->renderer->renderArrayActorsOmittedTiles[x][y].clear();
    }
  }
}

