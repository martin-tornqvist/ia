#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>
#include <stdexcept>

#include "Feature.h"
#include "ActorData.h"

using namespace std;

class Engine;

enum TurnType_t {
	turnType_fast,
	turnType_normal_1,
	turnType_slow,
	turnType_fastest,
	turnType_normal_2,
	endOfTurnType
};

class GameTime {
public:
	GameTime(Engine* engine) :
		currentTurnTypePos_(0),
		currentActorVectorPos_(0),
		turn_(0), eng(engine) {
		actors_.resize(0);
		featureMobs_.resize(0);
	}

	~GameTime();

	void insertActorInLoop(Actor* actor); //To insert at random position

	void letNextAct();

	int getTurn() {
		return turn_;
	}

	Actor* getCurrentActor() {
		return actors_.at(currentActorVectorPos_);
	}

	unsigned int getLoopSize() const {
		return actors_.size();
	}

	Actor* getActorAt(const unsigned int i) {
		return actors_.at(i);
	}

	void eraseElement(const unsigned int i) {
		if(actors_.empty() == false) {
			actors_.erase(actors_.begin() + i);
		}
	}

	vector<FeatureMob*> getFeatureMobsAtPos(const coord& pos);

	void addFeatureMob(FeatureMob* const feature) {
		featureMobs_.push_back(feature);
	}

	unsigned int getFeatureMobsSize() {
		return featureMobs_.size();
	}

	FeatureMob* getFeatureMobAt(const unsigned int i) {
		return featureMobs_.at(i);
	}

	void eraseAllFeatureMobs() {
		for(unsigned int i = 0; i < featureMobs_.size(); i++) {
			delete featureMobs_.at(i);
		}
		featureMobs_.resize(0);
	}

	void eraseFeatureMob(FeatureMob* const feature, const bool DESTROY_OBJECT) {
		int index = -1;
		const unsigned int SIZE = featureMobs_.size();
		for(unsigned int i = 0; i < SIZE; i++) {
			if(featureMobs_.at(i) == feature) {
				index = i;
				i = 9999;
			}
		}
		if(DESTROY_OBJECT) {
			delete featureMobs_.at(index);
		}
		featureMobs_.erase(featureMobs_.begin() + index);
	}

	void resetTurnTypeAndActorCounters() {
		currentTurnTypePos_ = 0;
		currentActorVectorPos_ = 0;
	}

private:
	friend class Renderer;
	friend class Dungeon;
	friend class DungeonFeatureFactory;

	void runNewTurnEvents();

	vector<ActorSpeed_t> turnTypeVector_;
	int currentTurnTypePos_;
	vector<Actor*> actors_;
	vector<FeatureMob*> featureMobs_;
	int currentActorVectorPos_;

	int turn_;

	Engine* eng;
};

#endif
