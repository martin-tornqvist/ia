#include "FeatureFactory.h"

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
#include "FeatureGrave.h"
#include "FeatureWall.h"

using namespace std;

Feature* FeatureFactory::spawnFeatureAt(const Feature_t id, const Pos pos,
                                        FeatureSpawnData* spawnData) {
  const FeatureData* const data = eng->featureDataHandler->getData(id);

  //General (simple) features
  if(data->spawnType == featureSpawnType_static) {
    if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
    FeatureStatic* feature = new FeatureStatic(id, pos, eng);
    replaceStaticFeatureAt(feature, pos);
    return feature;
  }
  if(data->spawnType == featureSpawnType_mob) {
    if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
    FeatureMob* feature = new FeatureMob(id, pos, eng);
    eng->gameTime->addFeatureMob(feature);
    return feature;
  }

  //Features with specific class
  switch(id) {
    case feature_door: {
      if(spawnData->getFeatureSpawnDataType() != featureSpawnData_door) {
        throw runtime_error("Expected door spawn data");
      }
      Door* door =
        new Door(id, pos, eng, dynamic_cast<DoorSpawnData*>(spawnData));
      replaceStaticFeatureAt(door, pos);
      delete spawnData;
      return door;
    }
    case feature_lever: {
      if(spawnData->getFeatureSpawnDataType() != featureSpawnData_lever) {
        throw runtime_error("Expected lever spawn data");
      }
      FeatureLever* lever =
        new FeatureLever(
        id, pos, eng, dynamic_cast<LeverSpawnData*>(spawnData));
      replaceStaticFeatureAt(lever, pos);
      delete spawnData;
      return lever;
    }
    case feature_trap: {
      if(spawnData->getFeatureSpawnDataType() != featureSpawnData_trap) {
        throw runtime_error("Expected trap spawn data");
      }
      Trap* trap =
        new Trap(id, pos, eng, dynamic_cast<TrapSpawnData*>(spawnData));
      replaceStaticFeatureAt(trap, pos);
      delete spawnData;
      return trap;
    }
    case feature_litDynamite: {
      if(spawnData->getFeatureSpawnDataType() != featureSpawnData_dynamite) {
        throw runtime_error("Expected dynamite spawn data");
      }
      LitDynamite* dynamite =
        new LitDynamite(
        id, pos, eng, dynamic_cast<DynamiteSpawnData*>(spawnData));
      eng->gameTime->addFeatureMob(dynamite);
      delete spawnData;
      return dynamite;
    }
    case feature_litFlare: {
      if(spawnData->getFeatureSpawnDataType() != featureSpawnData_dynamite) {
        throw runtime_error("Expected dynamite spawn data");
      }
      LitFlare* flare =
        new LitFlare(
        id, pos, eng, dynamic_cast<DynamiteSpawnData*>(spawnData));
      eng->gameTime->addFeatureMob(flare);
      delete spawnData;
      return flare;
    }
    case feature_smoke: {
      if(spawnData->getFeatureSpawnDataType() != featureSpawnData_smoke) {
        throw runtime_error("Expected smoke spawn data");
      }
      Smoke* smoke =
        new Smoke(id, pos, eng, dynamic_cast<SmokeSpawnData*>(spawnData));
      eng->gameTime->addFeatureMob(smoke);
      delete spawnData;
      return smoke;
    }
    case feature_proxEventWallCrumble: {
      if(spawnData->getFeatureSpawnDataType() !=
          featureSpawnData_proxEventWallCrumble) {
        throw runtime_error("Expected prox event wall crumble spawn data");
      }
      ProxEventWallCrumble* proxEvent =
        new ProxEventWallCrumble(
        id, pos, eng, dynamic_cast<ProxEventWallCrumbleSpawnData*>(spawnData));
      eng->gameTime->addFeatureMob(proxEvent);
      delete spawnData;
      return proxEvent;
    }
    case feature_tomb: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      Tomb* tomb = new Tomb(id, pos, eng);
      replaceStaticFeatureAt(tomb, pos);
      return tomb;
    }
//    case feature_pillarCarved: {
//        if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
//        CarvedPillar* pillar = new CarvedPillar(id, pos, eng);
//        replaceStaticFeatureAt(pillar, pos);
//        return pillar;
//      }
//      break;
//    case feature_barrel: {
//        if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
//        Barrel* barrel = new Barrel(id, pos, eng);
//        replaceStaticFeatureAt(barrel, pos);
//        return barrel;
//      }
//      break;
    case feature_cabinet: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      Cabinet* cabinet = new Cabinet(id, pos, eng);
      replaceStaticFeatureAt(cabinet, pos);
      return cabinet;
    }
    case feature_chest: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      Chest* chest = new Chest(id, pos, eng);
      replaceStaticFeatureAt(chest, pos);
      return chest;
    }
    case feature_fountain: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      Fountain* fountain = new Fountain(id, pos, eng);
      replaceStaticFeatureAt(fountain, pos);
      return fountain;
    }
    case feature_cocoon: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      Cocoon* cocoon = new Cocoon(id, pos, eng);
      replaceStaticFeatureAt(cocoon, pos);
      return cocoon;
    }
//    case feature_altar: {
//        if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
//        Altar* altar = new Altar(id, pos, eng);
//        replaceStaticFeatureAt(altar, pos);
//
//        return altar;
//      }
//      break;
    case feature_shallowMud:
    case feature_shallowWater:
    case feature_poolBlood: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      FeatureLiquidShallow* liquid = new FeatureLiquidShallow(id, pos, eng);
      replaceStaticFeatureAt(liquid, pos);
      return liquid;
    }
    case feature_deepWater: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      FeatureLiquidDeep* liquid = new FeatureLiquidDeep(id, pos, eng);
      replaceStaticFeatureAt(liquid, pos);
      return liquid;
    }
    case feature_gravestone: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
      Grave* grave = new Grave(id, pos, eng);
      replaceStaticFeatureAt(grave, pos);
      return grave;
    }
    case feature_stoneWall: {
      if(spawnData != NULL) {throw runtime_error("Expected NULL spawnData");}
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

  if((eng->map->featuresStatic[pos.x][pos.y]) != NULL) {
    delete eng->map->featuresStatic[pos.x][pos.y];
  }
  eng->map->featuresStatic[pos.x][pos.y] = newFeature;
}

