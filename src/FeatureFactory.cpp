#include "FeatureFactory.h"

#include <cassert>

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

Feature* FeatureFactory::spawnFeatureAt(const Feature_t id, const coord pos, FeatureSpawnData* spawnData) {
  const FeatureDef* const def = eng->featureData->getFeatureDef(id);

  //General (simple) features
  if(def->spawnType == featureSpawnType_static) {
    assert(spawnData == NULL);
    FeatureStatic* feature = new FeatureStatic(id, pos, eng);
    replaceStaticFeatureAt(feature, pos);
    assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
    return feature;
  }
  if(def->spawnType == featureSpawnType_mob) {
    assert(spawnData == NULL);
    FeatureMob* feature = new FeatureMob(id, pos, eng);
    eng->gameTime->addFeatureMob(feature);
    return feature;
  }

  //Features with specific class
  switch(id) {
    case feature_door: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_door);
        Door* door = new Door(id, pos, eng, dynamic_cast<DoorSpawnData*>(spawnData));
        replaceStaticFeatureAt(door, pos);
        delete spawnData;
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return door;
      }
    case feature_lever: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_lever);
        FeatureLever* lever = new FeatureLever(id, pos, eng, dynamic_cast<LeverSpawnData*>(spawnData));
        replaceStaticFeatureAt(lever, pos);
        delete spawnData;
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return lever;
      }
    case feature_trap: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_trap);
        Trap* trap = new Trap(id, pos, eng, dynamic_cast<TrapSpawnData*>(spawnData));
        replaceStaticFeatureAt(trap, pos);
        delete spawnData;
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return trap;
      }
      break;
    case feature_litDynamite: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_dynamite);
        LitDynamite* dynamite = new LitDynamite(id, pos, eng, dynamic_cast<DynamiteSpawnData*>(spawnData));
        eng->gameTime->addFeatureMob(dynamite);
        delete spawnData;
        return dynamite;
      }
      break;
    case feature_litFlare: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_dynamite);
        LitFlare* flare = new LitFlare(id, pos, eng, dynamic_cast<DynamiteSpawnData*>(spawnData));
        eng->gameTime->addFeatureMob(flare);
        delete spawnData;
        return flare;
      }
      break;
    case feature_smoke: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_smoke);
        Smoke* smoke = new Smoke(id, pos, eng, dynamic_cast<SmokeSpawnData*>(spawnData));
        eng->gameTime->addFeatureMob(smoke);
        delete spawnData;
        return smoke;
      }
      break;
    case feature_proxEventWallCrumble: {
        assert(spawnData != NULL);
        assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_proxEventWallCrumble);
        ProxEventWallCrumble* proxEvent = new ProxEventWallCrumble(id, pos, eng, dynamic_cast<ProxEventWallCrumbleSpawnData*>(spawnData));
        eng->gameTime->addFeatureMob(proxEvent);
        delete spawnData;
        return proxEvent;
      }
      break;
    case feature_tomb: {
        assert(spawnData == NULL);
        Tomb* tomb = new Tomb(id, pos, eng);
        replaceStaticFeatureAt(tomb, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return tomb;
      }
      break;
//    case feature_pillarCarved: {
//        assert(spawnData == NULL);
//        CarvedPillar* pillar = new CarvedPillar(id, pos, eng);
//        replaceStaticFeatureAt(pillar, pos);
//        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
//        return pillar;
//      }
//      break;
//    case feature_barrel: {
//        assert(spawnData == NULL);
//        Barrel* barrel = new Barrel(id, pos, eng);
//        replaceStaticFeatureAt(barrel, pos);
//        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
//        return barrel;
//      }
//      break;
    case feature_cabinet: {
        assert(spawnData == NULL);
        Cabinet* cabinet = new Cabinet(id, pos, eng);
        replaceStaticFeatureAt(cabinet, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return cabinet;
      }
      break;
    case feature_chest: {
        assert(spawnData == NULL);
        Chest* chest = new Chest(id, pos, eng);
        replaceStaticFeatureAt(chest, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return chest;
      }
      break;
    case feature_cocoon: {
        assert(spawnData == NULL);
        Cocoon* cocoon = new Cocoon(id, pos, eng);
        replaceStaticFeatureAt(cocoon, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return cocoon;
      }
      break;
//    case feature_altar: {
//        assert(spawnData == NULL);
//        Altar* altar = new Altar(id, pos, eng);
//        replaceStaticFeatureAt(altar, pos);
//        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
//        return altar;
//      }
//      break;
    case feature_shallowMud:
    case feature_shallowWater:
    case feature_poolBlood: {
        assert(spawnData == NULL);
        FeatureLiquidShallow* liquid = new FeatureLiquidShallow(id, pos, eng);
        replaceStaticFeatureAt(liquid, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
      }
      break;
    case feature_deepWater: {
        assert(spawnData == NULL);
        FeatureLiquidDeep* liquid = new FeatureLiquidDeep(id, pos, eng);
        replaceStaticFeatureAt(liquid, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
      }
    case feature_gravestone: {
        assert(spawnData == NULL);
        Grave* grave = new Grave(id, pos, eng);
        replaceStaticFeatureAt(grave, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return grave;
      }
    case feature_stoneWall: {
        assert(spawnData == NULL);
        Wall* wall = new Wall(id, pos, eng);
        replaceStaticFeatureAt(wall, pos);
        assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
        return wall;
      }
    default: {
      }
      break;
  }

  return NULL;
}

void FeatureFactory::replaceStaticFeatureAt(FeatureStatic* const newFeature, const coord pos) {
  if((eng->map->featuresStatic[pos.x][pos.y]) != NULL) {
    delete(eng->map->featuresStatic[pos.x][pos.y]);
  }
  eng->map->featuresStatic[pos.x][pos.y] = newFeature;
  assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == newFeature->getId());
}

