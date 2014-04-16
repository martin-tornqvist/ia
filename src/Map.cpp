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
#include "Utils.h"

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

  playerVisualMemory.clear();
}

Map::Map(Engine& engine) : eng(engine), dlvl_(0) {
  rooms.resize(0);

  eng.actorFactory->deleteAllMonsters();

  resetCells(false);
  eng.gameTime->eraseAllFeatureMobs();
  eng.gameTime->resetTurnTypeAndActorCounters();
}

Map::~Map() {
  resetMap();
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      delete cells[x][y].featureStatic;
    }
  }
}

//TODO This should probably go in a virtual method in Feature instead
void Map::switchToDestroyedFeatAt(const Pos& pos) {
  if(Utils::isPosInsideMap(pos)) {

    const FeatureId OLD_FEATURE_ID =
      eng.map->cells[pos.x][pos.y].featureStatic->getId();

    const vector<FeatureId> convertionCandidates =
      eng.featureDataHandler->getData(OLD_FEATURE_ID)->featuresOnDestroyed;

    const int SIZE = convertionCandidates.size();
    if(SIZE > 0) {
      const FeatureId NEW_ID =
        convertionCandidates.at(Rnd::dice(1, SIZE) - 1);

      eng.featureFactory->spawnFeatureAt(NEW_ID, pos);

      //Destroy adjacent doors?
      if(
        (NEW_ID == feature_rubbleHigh || NEW_ID == feature_rubbleLow) &&
        NEW_ID != OLD_FEATURE_ID) {
        for(int x = pos.x - 1; x <= pos.x + 1; x++) {
          for(int y = pos.y - 1; y <= pos.y + 1; y++) {
            if(x == 0 || y == 0) {
              const FeatureStatic* const f =
                eng.map->cells[x][y].featureStatic;
              if(f->getId() == feature_door) {
                eng.featureFactory->spawnFeatureAt(
                  feature_rubbleLow, Pos(x, y));
              }
            }
          }
        }
      }

      if(NEW_ID == feature_rubbleLow && NEW_ID != OLD_FEATURE_ID) {
        if(Rnd::percentile() < 50) {
          eng.itemFactory->spawnItemOnMap(ItemId::rock, pos);
        }
      }
    }
  }
}

void Map::resetMap() {
  eng.actorFactory->deleteAllMonsters();

  for(Room * room : rooms) {delete room;}

  rooms.resize(0);

  resetCells(true);
  eng.gameTime->eraseAllFeatureMobs();
  eng.gameTime->resetTurnTypeAndActorCounters();
}

void Map::resetCells(const bool MAKE_STONE_WALLS) {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {

      cells[x][y].clear();

      cells[x][y].pos = Pos(x, y);

      Renderer::renderArray[x][y].clear();
      Renderer::renderArrayNoActors[x][y].clear();

      if(MAKE_STONE_WALLS) {
        eng.featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      }
    }
  }
}

void Map::updateVisualMemory() {
  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      eng.map->cells[x][y].playerVisualMemory =
        Renderer::renderArrayNoActors[x][y];
    }
  }
}
