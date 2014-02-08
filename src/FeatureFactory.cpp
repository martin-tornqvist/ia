#include "FeatureFactory.h"

#include <assert.h>

#include "Engine.h"

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

#ifdef DEMO_MODE
#include "Renderer.h"
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

Feature* FeatureFactory::spawnFeatureAt(const FeatureId id, const Pos pos,
                                        FeatureSpawnData* spawnData) {
  const FeatureData* const data = eng.featureDataHandler->getData(id);

  //General (simple) features
  if(data->spawnType == featureSpawnType_static) {
    assert(spawnData == NULL);
    FeatureStatic* feature = new FeatureStatic(id, pos, eng);
    replaceStaticFeatureAt(feature, pos);
    return feature;
  }
  if(data->spawnType == featureSpawnType_mob) {
    assert(spawnData == NULL);
    FeatureMob* feature = new FeatureMob(id, pos, eng);
    eng.gameTime->addFeatureMob(feature);
    return feature;
  }

  //Features with specific class
  switch(id) {
    case feature_door: {
      assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_door);
      Door* door =
        new Door(id, pos, eng, dynamic_cast<DoorSpawnData*>(spawnData));
      replaceStaticFeatureAt(door, pos);
      delete spawnData;
      return door;
    }
    case feature_lever: {
      assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_lever);
      FeatureLever* lever =
        new FeatureLever(
        id, pos, eng, dynamic_cast<LeverSpawnData*>(spawnData));
      replaceStaticFeatureAt(lever, pos);
      delete spawnData;
      return lever;
    }
    case feature_trap: {
      assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_trap);
      Trap* trap =
        new Trap(id, pos, eng, dynamic_cast<TrapSpawnData*>(spawnData));
      replaceStaticFeatureAt(trap, pos);
      delete spawnData;
      return trap;
    }
    case feature_litDynamite: {
      assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_dynamite);
      LitDynamite* dynamite =
        new LitDynamite(
        id, pos, eng, dynamic_cast<DynamiteSpawnData*>(spawnData));
      eng.gameTime->addFeatureMob(dynamite);
      delete spawnData;
      return dynamite;
    }
    case feature_litFlare: {
      assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_dynamite);
      LitFlare* flare =
        new LitFlare(
        id, pos, eng, dynamic_cast<DynamiteSpawnData*>(spawnData));
      eng.gameTime->addFeatureMob(flare);
      delete spawnData;
      return flare;
    }
    case feature_smoke: {
      assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_smoke);
      Smoke* smoke =
        new Smoke(id, pos, eng, dynamic_cast<SmokeSpawnData*>(spawnData));
      eng.gameTime->addFeatureMob(smoke);
      delete spawnData;
      return smoke;
    }
    case feature_proxEventWallCrumble: {
      assert(spawnData->getFeatureSpawnDataType() ==
             featureSpawnData_proxEventWallCrumble);
      ProxEventWallCrumble* proxEvent =
        new ProxEventWallCrumble(
        id, pos, eng, dynamic_cast<ProxEventWallCrumbleSpawnData*>(spawnData));
      eng.gameTime->addFeatureMob(proxEvent);
      delete spawnData;
      return proxEvent;
    }
    case feature_tomb: {
      assert(spawnData == NULL);
      Tomb* tomb = new Tomb(id, pos, eng);
      replaceStaticFeatureAt(tomb, pos);
      return tomb;
    }
//    case feature_pillarCarved: {
//        assert(spawnData == NULL);
//        CarvedPillar* pillar = new CarvedPillar(id, pos, eng);
//        replaceStaticFeatureAt(pillar, pos);
//        return pillar;
//      }
//      break;
//    case feature_barrel: {
//        assert(spawnData == NULL);
//        Barrel* barrel = new Barrel(id, pos, eng);
//        replaceStaticFeatureAt(barrel, pos);
//        return barrel;
//      }
//      break;
    case feature_cabinet: {
      assert(spawnData == NULL);
      Cabinet* cabinet = new Cabinet(id, pos, eng);
      replaceStaticFeatureAt(cabinet, pos);
      return cabinet;
    }
    case feature_chest: {
      assert(spawnData == NULL);
      Chest* chest = new Chest(id, pos, eng);
      replaceStaticFeatureAt(chest, pos);
      return chest;
    }
    case feature_fountain: {
      assert(spawnData == NULL);
      Fountain* fountain = new Fountain(id, pos, eng);
      replaceStaticFeatureAt(fountain, pos);
      return fountain;
    }
    case feature_cocoon: {
      assert(spawnData == NULL);
      Cocoon* cocoon = new Cocoon(id, pos, eng);
      replaceStaticFeatureAt(cocoon, pos);
      return cocoon;
    }
//    case feature_altar: {
//        assert(spawnData == NULL);
//        Altar* altar = new Altar(id, pos, eng);
//        replaceStaticFeatureAt(altar, pos);
//
//        return altar;
//      }
//      break;
    case feature_shallowMud:
    case feature_shallowWater:
    case feature_poolBlood: {
      assert(spawnData == NULL);
      FeatureLiquidShallow* liquid = new FeatureLiquidShallow(id, pos, eng);
      replaceStaticFeatureAt(liquid, pos);
      return liquid;
    }
    case feature_deepWater: {
      assert(spawnData == NULL);
      FeatureLiquidDeep* liquid = new FeatureLiquidDeep(id, pos, eng);
      replaceStaticFeatureAt(liquid, pos);
      return liquid;
    }
    case feature_gravestone: {
      assert(spawnData == NULL);
      Grave* grave = new Grave(id, pos, eng);
      replaceStaticFeatureAt(grave, pos);
      return grave;
    }
    case feature_stairs: {
      assert(spawnData == NULL);
      Stairs* stairs = new Stairs(id, pos, eng);
      replaceStaticFeatureAt(stairs, pos);
      return stairs;
    }
    case feature_stoneWall: {
      assert(spawnData == NULL);
      Wall* wall = new Wall(id, pos, eng);
      replaceStaticFeatureAt(wall, pos);
      return wall;
    }
    default: {} break;
  }

  return NULL;
}

void FeatureFactory::replaceStaticFeatureAt(
  FeatureStatic* const newFeature, const Pos& pos) {

  Cell& cell = eng.map->cells[pos.x][pos.y];

  FeatureStatic* const oldFeature = cell.featureStatic;

  if(oldFeature != NULL) {
    delete oldFeature;
  }
  cell.featureStatic = newFeature;
}

