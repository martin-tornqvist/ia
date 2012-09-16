#include "GameTime.h"

#include "Engine.h"

#include "ConstDungeonSettings.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "PlayerVisualMemory.h"
#include "Map.h"
#include "Populate.h"

vector<FeatureMob*> GameTime::getFeatureMobsAtPos(const coord& pos) {
	vector<FeatureMob*> returnVector;
	for(unsigned int i = 0; i < featureMobs_.size(); i++) {
		if(featureMobs_.at(i)->getPos() == pos) {
			returnVector.push_back(featureMobs_.at(i));
		}
	}
	return returnVector;
}

GameTime::~GameTime() {
	for(unsigned int i = 0; i < actors_.size(); i++) {
		delete actors_.at(i);
	}
	actors_.resize(0);

	for(unsigned int i = 0; i < featureMobs_.size(); i++) {
		delete featureMobs_.at(i);
	}
	featureMobs_.resize(0);
}

void GameTime::insertActorInLoop(Actor* actor) {
	actors_.insert(actors_.begin() + eng->dice.getInRange(0, actors_.size()), actor);
}

/*
 * For every turn type step, iterate through all actors and let those who can act
 * during this type of turn act. When all actors who can act during this phase have
 * acted, if this is a normal speed phase - consider it a global new turn; update
 * status effects, update timed features, spawn more monsters etc.
 */
void GameTime::letNextAct() {
	Actor* currentActor = getCurrentActor();

	if(currentActor == eng->player) {
		// Player turn ends
		eng->player->insanityShortTemp = 0;
		eng->playerVisualMemory->updateVisualMemory();
		eng->player->FOVupdate();
		eng->specialRoomHandler->runRoomEvents();
		eng->player->setTempShockFromFeatures();
	} else {
		// Monster turn ends
		Monster* monster = dynamic_cast<Monster*>(currentActor);
		if(monster->playerAwarenessCounter > 0) {
		    monster->playerAwarenessCounter -= 1;
		}
		monster->clearHeardSounds();
	}

	TurnType_t currentTurnType = static_cast<TurnType_t>(currentTurnTypePos_);

	bool actorWhoCanActThisTurnFound = false;
	while(actorWhoCanActThisTurnFound == false) {
		currentTurnType = static_cast<TurnType_t>(currentTurnTypePos_);

		currentActorVectorPos_++;

		if(static_cast<unsigned int>(currentActorVectorPos_) >= actors_.size()) {
			currentActorVectorPos_ = 0;
			currentTurnTypePos_++;
			if(currentTurnTypePos_ == endOfTurnType) {
				currentTurnTypePos_ = 0;
			}
			if(currentTurnType != turnType_fast && currentTurnType != turnType_fastest) {
				runNewTurnEvents();
			}
		}

		currentActor = getCurrentActor();

		const bool IS_SLOWED = currentActor->getStatusEffectsHandler()->hasEffect(statusSlowed);
		const ActorSpeed_t defSpeed = currentActor->getInstanceDefinition()->speed;
		const ActorSpeed_t realSpeed = IS_SLOWED == false || defSpeed == actorSpeed_sluggish ? defSpeed : static_cast<ActorSpeed_t>(defSpeed - 1);
		switch(realSpeed) {
		case actorSpeed_sluggish: {
			actorWhoCanActThisTurnFound = (currentTurnType == turnType_slow || currentTurnType == turnType_normal_2) && eng->dice(1,100) < 65;
		}
		break;
		case actorSpeed_slow: {
			actorWhoCanActThisTurnFound = currentTurnType == turnType_slow || currentTurnType == turnType_normal_2;
		}
		break;
		case actorSpeed_normal: {
			actorWhoCanActThisTurnFound = currentTurnType != turnType_fast && currentTurnType != turnType_fastest;
		}
		break;
		case actorSpeed_fast: {
			actorWhoCanActThisTurnFound = currentTurnType != turnType_fastest;
		}
		break;
		case actorSpeed_fastest: {
			actorWhoCanActThisTurnFound = true;
		}
		break;
		}
	}

	// Player turn begins?
	if(currentActor == eng->player) {
		eng->player->newTurn();
		eng->renderer->drawMapAndInterface();
	}
}

void GameTime::runNewTurnEvents() {
	turn_++;

	eng->basicUtils->resetBoolArray(eng->map->light, false);

	Actor* actor = NULL;
	unsigned int loopSize = actors_.size();
	for(unsigned int i = 0; i < loopSize; i++) {
		actor = actors_.at(i);
		//Update status effects on all actors, this also makes the monster player-aware if
		//it has any active status effect.
		actor->getStatusEffectsHandler()->newTurnAllEffects();

		//Delete dead, mangled actors
		if(actor->deadState == actorDeadState_mangled) {
			delete actor;
			if(eng->player->target == actor) {
				eng->player->target = NULL;
			}
			actors_.erase(actors_.begin() + i);
			i--;
			loopSize--;
			if(static_cast<unsigned int>(currentActorVectorPos_) >= actors_.size()) {
				currentActorVectorPos_ = 0;
			}
		}
	}
	//Update all timed features (both mobs contained here, and statics contained in Dungeon)
	for(unsigned int i = 0; i < featureMobs_.size(); i++) {
		featureMobs_.at(i)->getLight(eng->map->light);
		featureMobs_.at(i)->newTurn(); //Note: this may erase the feature
	}
	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			eng->map->featuresStatic[x][y]->getLight(eng->map->light);
			eng->map->featuresStatic[x][y]->newTurn();
		}
	}

	//Spawn more monsters? (If an unexplored cell is selected, the spawn is aborted)
	if(eng->map->getDungeonLevel() >= 1 && eng->map->getDungeonLevel() <= LAST_CAVERN_LEVEL) {
		const int SPAWN_N_TURN = 55;
		if(turn_ == (turn_ / SPAWN_N_TURN) * SPAWN_N_TURN) {
			eng->populate->spawnOneMonster(true);
		}
	}
}

Actor* GameTime::getCurrentActor() {
//   const int VECTOR_SIZE = actors_.size();
   return actors_.at(currentActorVectorPos_);
}
