#ifndef FEATURE_PROX_EVENT_H
#define FEATURE_PROX_EVENT_H

#include "Feature.h"
#include "FeatureFactory.h"

class ProxEvent: public FeatureMob {
public:
	~ProxEvent() {
	}

	void newTurn();

protected:
	ProxEvent(Feature_t id, coord pos, Engine* engine) :
		FeatureMob(id, pos, engine) {}

	virtual void playerIsNear() = 0;
};

class ProxEventWallCrumble: public ProxEvent {
public:

private:
	friend class FeatureFactory;
	ProxEventWallCrumble(Feature_t id, coord pos, Engine* engine, ProxEventWallCrumbleSpawnData* spawnData) :
		ProxEvent(id, pos, engine), wallCells_(spawnData->wallCells_), innerCells_(spawnData->innerCells_) {
	}

	void playerIsNear();

	vector<coord> wallCells_;
	vector<coord> innerCells_;
};




#endif
