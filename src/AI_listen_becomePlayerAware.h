#ifndef AI_LISTEN_BECOME_PLAYER_AWARE
#define AI_LISTEN_BECOME_PLAYER_AWARE

#include "Engine.h"
#include "Actor.h"

class AI_listen_becomePlayerAware {
public:
	static void learn(Monster* monster, const vector<Sound>& soundsHeard) {
		if(monster->deadState == actorDeadState_alive && monster->isRoamingAllowed) {
			if(soundsHeard.size() != 0) {
				for(unsigned int i = 0; i < soundsHeard.size(); i++) {
					if(soundsHeard.at(i).getIsAlertingMonsters()) {
						monster->playerAwarenessCounter = monster->getInstanceDefinition()->nrTurnsAwarePlayer;
					}
				}
			}
		}
	}

private:

};

#endif

