#include "FeatureLitDynamite.h"

#include "Engine.h"
#include "Explosion.h"
#include "Map.h"

void LitDynamite::newTurn() {
	turnsLeftToExplosion_--;
	if(turnsLeftToExplosion_ <= 0) {
		eng->explosionMaker->runExplosion(pos_);
		eng->gameTime->eraseFeatureMob(this, true);
	}
}

void LitFlare::newTurn() {
	life_--;
	if(life_ <= 0) {
		eng->gameTime->eraseFeatureMob(this, true);
	}
}

LitFlare::LitFlare(Feature_t id, coord pos, Engine* engine, DynamiteSpawnData* spawnData) :
	FeatureMob(id, pos, engine), life_(spawnData->turnsLeftToExplosion_) {

	bool lightToStore[MAP_X_CELLS][MAP_Y_CELLS];
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->basicUtils->resetBoolArray(lightToStore, false);
	eng->basicUtils->resetBoolArray(blockers, false);

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			blockers[x][y] = eng->map->darkness[x][y] == false;
		}
	}

	int floodFillValues[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeFloodFill(pos_, blockers, floodFillValues, 99999, coord(-1, -1));

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			if(floodFillValues[x][y] > 0) {
				lightToStore[x][y] = true;
			}
		}
	}
	for(int dy = -1; dy <= 1; dy++) {
		for(int dx = -1; dx <= 1; dx++) {
			lightToStore[pos_.x][pos_.y] = true;
		}
	}

	eng->mapTests->makeMapVectorFromArray(lightToStore, light_);
}

void LitFlare::getLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {
	const int SIZE = static_cast<int> (light_.size());
	for(int i = 0; i < SIZE; i++) {
		const coord c = light_.at(i);
		light[c.x][c.y] = true;
	}
}
