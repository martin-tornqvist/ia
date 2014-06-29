#include "Init.h"

#include "FeatureFactory.h"

#include <assert.h>

#include "FeatureDoor.h"
#include "FeatureLever.h"
#include "FeatureLitDynamite.h"
#include "FeatureTrap.h"
#include "FeatureSmoke.h"
#include "FeatureProxEvent.h"
#include "Map.h"
#include "FeatureExaminable.h"
#include "FeatureLiquid.h"
#include "FeatureWall.h"
#include "Utils.h"

#ifdef DEMO_MODE
#include "Renderer.h"
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

namespace FeatureFactory {

namespace {

void replaceStaticFeatureAt(FeatureStatic* const newFeature, const Pos& pos) {
  Cell& cell = Map::cells[pos.x][pos.y];

  FeatureStatic* const oldFeature = cell.featureStatic;

  if(oldFeature) {delete oldFeature;}

  cell.featureStatic = newFeature;

#ifdef DEMO_MODE
  if(newFeature->getId() == FeatureId::floor) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        Map::cells[x][y].isSeenByPlayer = Map::cells[x][y].isExplored = true;
      }
    }
    Renderer::drawMap();
    Renderer::drawGlyph('X', Panel::map, pos, clrYellow);
    Renderer::updateScreen();
    SdlWrapper::sleep(2); //Note: Delay must be >= 2 for user input to be read
  }
#endif // DEMO_MODE
}

} //namespace

Feature* mk(const FeatureId id, const Pos pos, FeatureSpawnData* spawnData) {
  assert(Utils::isPosInsideMap(pos));

  const FeatureDataT* const data = FeatureData::getData(id);

  //General (simple) features
  if(data->spawnType == featureSpawnType_static) {
    assert(!spawnData);
    FeatureStatic* feature = new FeatureStatic(id, pos);
    replaceStaticFeatureAt(feature, pos);
    return feature;
  }
  if(data->spawnType == featureSpawnType_mob) {
    assert(!spawnData);
    FeatureMob* feature = new FeatureMob(id, pos);
    GameTime::addFeatureMob(feature);
    return feature;
  }

  //Features with specific class
  switch(id) {
    case FeatureId::door: {
      assert(spawnData->getFeatureSpawnDataType() == FeatureSpawnDataId::door);
      Door* door =
        new Door(id, pos, static_cast<DoorSpawnData*>(spawnData));
      replaceStaticFeatureAt(door, pos);
      delete spawnData;
      return door;
    }
    case FeatureId::lever: {
      assert(spawnData->getFeatureSpawnDataType() == FeatureSpawnDataId::lever);
      FeatureLever* lever =
        new FeatureLever(id, pos, static_cast<LeverSpawnData*>(spawnData));
      replaceStaticFeatureAt(lever, pos);
      delete spawnData;
      return lever;
    }
    case FeatureId::trap: {
      assert(spawnData->getFeatureSpawnDataType() == FeatureSpawnDataId::trap);
      Trap* trap = new Trap(id, pos, static_cast<TrapSpawnData*>(spawnData));
      replaceStaticFeatureAt(trap, pos);
      delete spawnData;
      return trap;
    }
    case FeatureId::litDynamite: {
      assert(spawnData->getFeatureSpawnDataType() ==
             FeatureSpawnDataId::dynamite);
      LitDynamite* dynamite =
        new LitDynamite(id, pos, static_cast<DynamiteSpawnData*>(spawnData));
      GameTime::addFeatureMob(dynamite);
      delete spawnData;
      return dynamite;
    }
    case FeatureId::litFlare: {
      assert(spawnData->getFeatureSpawnDataType() ==
             FeatureSpawnDataId::dynamite);
      LitFlare* flare =
        new LitFlare(id, pos, static_cast<DynamiteSpawnData*>(spawnData));
      GameTime::addFeatureMob(flare);
      delete spawnData;
      return flare;
    }
    case FeatureId::smoke: {
      assert(spawnData->getFeatureSpawnDataType() == FeatureSpawnDataId::smoke);
      Smoke* smoke =
        new Smoke(id, pos, static_cast<SmokeSpawnData*>(spawnData));
      GameTime::addFeatureMob(smoke);
      delete spawnData;
      return smoke;
    }
    case FeatureId::proxEventWallCrumble: {
      assert(spawnData->getFeatureSpawnDataType() ==
             FeatureSpawnDataId::proxEventWallCrumble);
      ProxEventWallCrumble* proxEvent =
        new ProxEventWallCrumble(
        id, pos, static_cast<ProxEventWallCrumbleSpawnData*>(spawnData));
      GameTime::addFeatureMob(proxEvent);
      delete spawnData;
      return proxEvent;
    }
    case FeatureId::tomb: {
      assert(!spawnData);
      Tomb* tomb = new Tomb(id, pos);
      replaceStaticFeatureAt(tomb, pos);
      return tomb;
    }
//    case FeatureId::pillarCarved: {
//        assert(!spawnData);
//        CarvedPillar* pillar = new CarvedPillar(id, pos);
//        replaceStaticFeatureAt(pillar, pos);
//        return pillar;
//      }
//      break;
//    case FeatureId::barrel: {
//        assert(!spawnData);
//        Barrel* barrel = new Barrel(id, pos);
//        replaceStaticFeatureAt(barrel, pos);
//        return barrel;
//      }
//      break;
    case FeatureId::cabinet: {
      assert(!spawnData);
      Cabinet* cabinet = new Cabinet(id, pos);
      replaceStaticFeatureAt(cabinet, pos);
      return cabinet;
    }
    case FeatureId::chest: {
      assert(!spawnData);
      Chest* chest = new Chest(id, pos);
      replaceStaticFeatureAt(chest, pos);
      return chest;
    }
    case FeatureId::fountain: {
      assert(!spawnData);
      Fountain* fountain = new Fountain(id, pos);
      replaceStaticFeatureAt(fountain, pos);
      return fountain;
    }
    case FeatureId::cocoon: {
      assert(!spawnData);
      Cocoon* cocoon = new Cocoon(id, pos);
      replaceStaticFeatureAt(cocoon, pos);
      return cocoon;
    }
//    case FeatureId::altar: {
//        assert(!spawnData);
//        Altar* altar = new Altar(id, pos);
//        replaceStaticFeatureAt(altar, pos);
//
//        return altar;
//      }
//      break;
    case FeatureId::shallowMud:
    case FeatureId::shallowWater:
    case FeatureId::poolBlood: {
      assert(!spawnData);
      FeatureLiquidShallow* liquid = new FeatureLiquidShallow(id, pos);
      replaceStaticFeatureAt(liquid, pos);
      return liquid;
    }
    case FeatureId::deepWater: {
      assert(!spawnData);
      FeatureLiquidDeep* liquid = new FeatureLiquidDeep(id, pos);
      replaceStaticFeatureAt(liquid, pos);
      return liquid;
    }
    case FeatureId::gravestone: {
      assert(!spawnData);
      Grave* grave = new Grave(id, pos);
      replaceStaticFeatureAt(grave, pos);
      return grave;
    }
    case FeatureId::stairs: {
      assert(!spawnData);
      Stairs* stairs = new Stairs(id, pos);
      replaceStaticFeatureAt(stairs, pos);
      return stairs;
    }
    case FeatureId::wall: {
      assert(!spawnData);
      Wall* wall = new Wall(id, pos);
      replaceStaticFeatureAt(wall, pos);
      return wall;
    }
    default: {} break;
  }

  return nullptr;
}

} //FeatureFactory
