#ifndef AI_SET_INTEREST_IF_SEE_TARGET_H
#define AI_SET_INTEREST_IF_SEE_TARGET_H

#include "Engine.h"
#include "ActorPlayer.h"

class AI_look_becomePlayerAware {
public:
	static void learn(Monster* monster, Engine* engine) {
		if(monster->deadState == actorDeadState_alive) {
			const int PLAYER_SNEAK = engine->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_sneaking, true);

			if(engine->abilityRoll->roll(PLAYER_SNEAK) <= failSmall || monster->playerAwarenessCounter > 0) {

				bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
				engine->mapTests->makeVisionBlockerArray(blockers);

				if(monster->checkIfSeeActor(*engine->player, blockers)) {
					monster->playerAwarenessCounter = monster->getInstanceDefinition()->nrTurnsAwarePlayer;
				}
			}
		}
	}

private:

};

#endif
