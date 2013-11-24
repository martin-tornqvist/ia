#include "Map.h"

#include "Engine.h"
#include "Feature.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Renderer.h"
#include "MapGen.h"

using namespace std;

Map::Map(Engine* engine) : eng(engine), dlvl_(0) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      //Note: FeatureFactory cannot be used at this point, since the
      //feature array is not yet initialized
      featuresStatic[x][y] =
        new FeatureStatic(feature_stoneWall, Pos(x, y), eng);
    }
  }

  rooms.resize(0);
  clearGrids(false);
}

Map::~Map() {
  clearMap();
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      delete featuresStatic[x][y];
    }
  }
}

//TODO This should probably go in a virtual method in Feature instead
void Map::switchToDestroyedFeatAt(const Pos pos) {
  if(eng->mapTests->isPosInsideMap(pos)) {

    const Feature_t OLD_FEATURE_ID =
      eng->map->featuresStatic[pos.x][pos.y]->getId();

    const vector<Feature_t> convertionCandidates =
      eng->featureDataHandler->getData(OLD_FEATURE_ID)->featuresOnDestroyed;

    const int SIZE = convertionCandidates.size();
    if(SIZE > 0) {
      const Feature_t NEW_ID =
        convertionCandidates.at(eng->dice(1, SIZE) - 1);

      eng->featureFactory->spawnFeatureAt(NEW_ID, pos);

      //Destroy adjacent doors?
      if(
        (NEW_ID == feature_rubbleHigh || NEW_ID == feature_rubbleLow) &&
        NEW_ID != OLD_FEATURE_ID) {
        for(int x = pos.x - 1; x <= pos.x + 1; x++) {
          for(int y = pos.y - 1; y <= pos.y + 1; y++) {
            if(x == 0 || y == 0) {
              if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
                eng->featureFactory->spawnFeatureAt(
                  feature_rubbleLow, Pos(x, y));
              }
            }
          }
        }
      }

      if(NEW_ID == feature_rubbleLow && NEW_ID != OLD_FEATURE_ID) {
        if(eng->dice.percentile() < 50) {
          eng->itemFactory->spawnItemOnMap(item_rock, pos);
        }
      }
    }
  }
}

void Map::clearMap() {
  eng->actorFactory->deleteAllMonsters();

  const int NR_ROOMS = rooms.size();
  for(int i = 0; i < NR_ROOMS; i++) {
    delete rooms.at(i);
  }
  rooms.resize(0);

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
        eng->featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));

        if(items[x][y] != NULL) {
          delete(items[x][y]);
        }
      }

      items[x][y] = NULL;
      darkness[x][y] = false;
      light[x][y] = false;
      playerVisualMemoryAscii[x][y].glyph = ' ';
      playerVisualMemoryAscii[x][y].color = clrBlack;
      playerVisualMemoryTiles[x][y].tile = tile_empty;
      playerVisualMemoryTiles[x][y].color = clrBlack;
      eng->renderer->renderArrayAscii[x][y].clear();
      eng->renderer->renderArrayTiles[x][y].clear();
      eng->renderer->renderArrayActorsOmittedAscii[x][y].clear();
      eng->renderer->renderArrayActorsOmittedTiles[x][y].clear();
    }
  }
}

