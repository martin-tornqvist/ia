#include "KnockBack.h"

#include "Engine.h"
#include "Attack.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Timer.h"

void KnockBack::attemptKnockBack(Actor* const defender, const coord& attackedFromPos, const bool IS_SPIKE_GUN) {
	if(defender->getInstanceDefinition()->actorSize <= actorSize_giant) {
		const bool DEFENDER_IS_MONSTER = defender != eng->player;

		const MoveType_t defenderMoveType = defender->getInstanceDefinition()->moveType;
		const bool WALKTYPE_CAN_BE_KNOCKED_BACK = defenderMoveType != moveType_ethereal && defenderMoveType != moveType_ooze;

		const coord delta = (defender->pos - attackedFromPos).getSigns();
		const int KNOCK_BACK_RANGE = IS_SPIKE_GUN ? eng->dice.getInRange(2, 3) : eng->dice(1, 2);

		for(int i = 0; i < KNOCK_BACK_RANGE; i++) {

			const coord c = defender->pos + delta;

			bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
			eng->mapTests->makeMoveBlockerArray(defender, blockers);
			const bool CELL_BLOCKED = blockers[c.x][c.y];
			const bool CELL_IS_BOTTOMLESS = eng->map->featuresStatic[c.x][c.y]->isBottomless();

			if(WALKTYPE_CAN_BE_KNOCKED_BACK && (CELL_BLOCKED == false || CELL_IS_BOTTOMLESS)) {
				if(i == 0) {
					if(IS_SPIKE_GUN == false) {
						defender->getStatusEffectsHandler()->attemptAddEffect(new StatusParalyzed(1), false, true);
					}

					if(DEFENDER_IS_MONSTER) {
						eng->log->addMessage(defender->getNameThe() + " is knocked back!");
					} else {
						eng->log->addMessage("You are knocked back!");
					}
				}

				defender->pos = c;

				eng->renderer->drawMapAndInterface();
				Timer t;
				t.start();
				while(t.get_ticks() < eng->config->DELAY_PROJECTILE_DRAW) {
				}

				if(CELL_IS_BOTTOMLESS) {
					if(DEFENDER_IS_MONSTER) {
						eng->log->addMessage(defender->getNameThe() + " plummets down the depths.", clrMessageGood);
					} else {
						eng->log->addMessage("You plummet down the depths!", clrMessageBad);
					}
					defender->die(true, false, false);
					i = 99999;
				}

			} else {
				// Defender nailed to a wall from a  spike gun?
				if(IS_SPIKE_GUN) {
					if(eng->map->featuresStatic[c.x][c.y]->isVisionPassable() == false) {
						defender->getStatusEffectsHandler()->attemptAddEffect(new StatusNailed(eng));
					}
				}

				i = 9999;
			}
		}
	}
}
