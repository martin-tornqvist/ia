#include "FeatureFactory.h"

#include <cassert>

#include "Engine.h"

#include "FeatureDoor.h"
#include "FeatureLitDynamite.h"
#include "FeatureTrap.h"
#include "FeatureSmoke.h"
#include "FeatureProxEvent.h"
#include "Map.h"
#include "FeatureVariousExaminable.h"

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
	switch (id) {
	case feature_door: {
		assert(spawnData != NULL);
		assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_door);
		Door* door = new Door(id, pos, eng, dynamic_cast<DoorSpawnData*> (spawnData));
		replaceStaticFeatureAt(door, pos);
		delete spawnData;
		assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
		return door;
	}
	case feature_trap: {
		assert(spawnData != NULL);
		assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_trap);
		Trap* trap = new Trap(id, pos, eng, dynamic_cast<TrapSpawnData*> (spawnData));
		replaceStaticFeatureAt(trap, pos);
		delete spawnData;
		assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == id);
		return trap;
	}
	break;
	case feature_litDynamite: {
		assert(spawnData != NULL);
		assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_dynamite);
		LitDynamite* dynamite = new LitDynamite(id, pos, eng, dynamic_cast<DynamiteSpawnData*> (spawnData));
		eng->gameTime->addFeatureMob(dynamite);
		delete spawnData;
		return dynamite;
	}
	break;
	case feature_litFlare: {
		assert(spawnData != NULL);
		assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_dynamite);
		LitFlare* flare = new LitFlare(id, pos, eng, dynamic_cast<DynamiteSpawnData*> (spawnData));
		eng->gameTime->addFeatureMob(flare);
		delete spawnData;
		return flare;
	}
	break;
	case feature_smoke: {
		assert(spawnData != NULL);
		assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_smoke);
		Smoke* smoke = new Smoke(id, pos, eng, dynamic_cast<SmokeSpawnData*> (spawnData));
		eng->gameTime->addFeatureMob(smoke);
		delete spawnData;
		return smoke;
	}
	break;
	case feature_proxEventWallCrumble: {
		assert(spawnData != NULL);
		assert(spawnData->getFeatureSpawnDataType() == featureSpawnData_proxEventWallCrumble);
		ProxEventWallCrumble* proxEvent = new ProxEventWallCrumble(id, pos, eng, dynamic_cast<ProxEventWallCrumbleSpawnData*> (spawnData));
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
	default: {
	}
	break;
	}

	return NULL;
}

void FeatureFactory::replaceStaticFeatureAt(FeatureStatic* const newFeature, const coord pos) {
	if((eng->map->featuresStatic[pos.x][pos.y]) != NULL) {
		delete (eng->map->featuresStatic[pos.x][pos.y]);
	}
	(eng->map->featuresStatic[pos.x][pos.y]) = newFeature;
	assert(eng->map->featuresStatic[pos.x][pos.y]->getId() == newFeature->getId());
}

