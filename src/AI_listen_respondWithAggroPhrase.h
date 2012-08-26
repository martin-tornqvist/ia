#ifndef AI_LISTEN_RESPOND_WITH_AGGRO_PHRASE
#define AI_LISTEN_RESPOND_WITH_AGGRO_PHRASE

#include "Engine.h"
#include "Actor.h"

/*
 * Purpose: Listen for sounds and respond with a phrase.
 */

class AI_listen_respondWithAggroPhrase {
public:
	static void learn(Actor* const monster, const vector<Sound>& soundsHeard, Engine* const engine) {
		if(monster->deadState == actorDeadState_alive && dynamic_cast<Monster*>(monster)->isRoamingAllowed == true) {
			if(engine->dice(1, 100) < 75) {
				if(soundsHeard.empty() == false) {
					const Sound* curSound = NULL;
					bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
					engine->mapTests->makeVisionBlockerArray(blockers);

					for(unsigned int i = 0; i < soundsHeard.size(); i++) {
						curSound = &(soundsHeard.at(i));

						if(curSound->getIsAlertingMonsters()) {
							const string* const phrase = engine->phrases->getRandomAggroPhrase(monster);
							if(phrase != NULL) {
								const bool SEEN_BY_PLAYER = engine->player->checkIfSeeActor(*monster, blockers);
								const string prePhrase = SEEN_BY_PLAYER == true ? "[" + monster->getNameThe() + "]" : "[Voice]";
								engine->soundEmitter->emitSound(Sound(prePhrase + *phrase, false, monster->pos, 3, false));
								break;
							}
						}
					}
				}
			}
		}
	}

private:

};

#endif
