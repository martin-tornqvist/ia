#ifndef FEATURE_DOOR_H
#define FEATURE_DOOR_H

#include <iostream>

#include "SDL/SDL.h"
#include "Feature.h"

using namespace std;

enum DoorSpawnState_t {
	doorSpawnState_open,
	doorSpawnState_closed,
	doorSpawnState_stuck,
	doorSpawnState_broken,
	doorSpawnState_secret,
	doorSpawnState_secretAndStuck
};

enum DoorMaterial_t {
   doorMaterial_wood,
   doorMaterial_metal
};

class Engine;
class DoorSpawnData;

//TODO When additional doortypes are introduced, see traps for an excellent way of implementing them
class Door: public FeatureStatic {
public:
	~Door() {
	}

	void bump(Actor* actorBumping);
	bool isMovePassable(Actor* const actorMoving) const;
	bool isMoveTypePassable(const MoveType_t moveType) const;
	bool isVisionPassable() const;
	bool isShootPassable() const;
	bool isSmokePassable() const;
	SDL_Color getColor() const;
	char getGlyph() const;
	Tile_t getTile() const;

	void tryOpen(Actor* actorTrying);
	void tryClose(Actor* actorTrying);
	void tryBash(Actor* actorTrying);
	bool trySpike(Actor* actorTrying);

	bool isOpen() const {
		return isOpen_;
	}
	bool isSecret() const {
		return isSecret_;
	}
	bool isStuck() const {
		return isStuck_;
	}

	string getDescription(const bool DEFINITE_ARTICLE) const;

   MaterialType_t getMaterialType() const;

	void reveal(const bool PRINT_MESSAGE);

	void setToSecret() {
		isOpen_ = false;
		isSecret_ = true;
	}

protected:
	friend class FeatureFactory;
	friend class MapBuildBSP;
	Door(Feature_t id, coord pos, Engine* engine, DoorSpawnData* spawnData);

	const FeatureDef* const mimicFeature_;
	int nrOfSpikes_;

	bool isOpen_, isBroken_, isStuck_, isSecret_;

	DoorMaterial_t material_;

	friend class Player;
	void playerTrySpotHidden();
};

#endif

