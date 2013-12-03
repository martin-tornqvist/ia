#include "Map.h"

#include "Engine.h"
#include "Feature.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Renderer.h"
#include "MapGen.h"
#include "Item.h"

using namespace std;

inline void Cell::clear() {
  isExplored = isSeenByPlayer = isLight = isDark = false;

  if(featureStatic != NULL) {
    delete featureStatic;
    featureStatic = NULL;
  }

  if(item != NULL) {
    delete item;
    item = NULL;
  }

  playerVisualMemoryAscii.clear();
  playerVisualMemoryTiles.clear();
}

Map::Map(Engine* engine) : eng(engine), dlvl_(0) {
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      //Note: FeatureFactory cannot be used at this point, since the
//      //feature array is not yet initialized
//      cells[x][y].featureStatic =
//        new FeatureStatic(feature_stoneWall, Pos(x, y), eng);
//    }
//  }

  rooms.resize(0);
  resetMap();
}

Map::~Map() {
  resetMap();
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      delete cells[x][y].featureStatic;
    }
  }
}

//TODO This should probably go in a virtual method in Feature instead
void Map::switchToDestroyedFeatAt(const Pos pos) {
  if(eng->basicUtils->isPosInsideMap(pos)) {

    const Feature_t OLD_FEATURE_ID =
      eng->map->cells[pos.x][pos.y].featureStatic->getId();

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
              const FeatureStatic* const f =
                eng->map->cells[x][y].featureStatic;
              if(f->getId() == feature_door) {
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

void Map::resetMap() {
  eng->actorFactory->deleteAllMonsters();

  const int NR_ROOMS = rooms.size();
  for(int i = 0; i < NR_ROOMS; i++) {
    delete rooms.at(i);
  }
  rooms.resize(0);

  resetCells(true);
  eng->gameTime->eraseAllFeatureMobs();
  eng->gameTime->resetTurnTypeAndActorCounters();
}

void Map::resetCells(const bool MAKE_STONE_WALLS) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      cells[x][y].clear();

      if(MAKE_STONE_WALLS) {
        eng->featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      }
    }
  }
}

