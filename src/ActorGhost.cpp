#include "monsterIncludeFiles.h"

#include "Log.h"

void Ghost::actorSpecificAct() {
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(AI_makeRoomForFriend::action(this, eng))
		return;

	if(eng->dice(1, 100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng))
			return;

	if(eng->dice(1, 100) < 65) {
		if(AI_castRandomSpellIfAware::action(this, eng))
			return;
	}

	if(deadState == actorDeadState_alive) {
		if(playerAwarenessCounter > 0) {
			if(eng->mapTests->isCellsNeighbours(pos, eng->player->pos, false)) {
				if(eng->dice(1, 100) < 35) {

					bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
					eng->mapTests->makeVisionBlockerArray(blockers);
					const bool PLAYER_SEES_ME = eng->player->checkIfSeeActor(*this, blockers);
					const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
					eng->log->addMessage(refer + " reaches for you... ");
					const AbilityRollResult_t rollResult = eng->abilityRoll->roll(
					      eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_dodge, true));
					const bool PLAYER_DODGES = rollResult >= successSmall;
					if(PLAYER_DODGES) {
						eng->log->addMessage("You dodge!", clrMessageGood);
					} else {
						bool deflectedByArmor = false;
						Item* const playerArmor = eng->player->getInventory()->getItemInSlot(slot_armorBody);
						if(playerArmor != NULL) {
							const int ARMOR_CHANCE_TO_DEFLECT = playerArmor->getInstanceDefinition().armorData.chanceToDeflectTouchAttacks;
							if(eng->dice(1, 100) < ARMOR_CHANCE_TO_DEFLECT) {
								deflectedByArmor = true;
							}
						}

						if(deflectedByArmor) {
							const string armorName = playerArmor->getInstanceDefinition().name.name;
							eng->log->addMessage("The touch is deflected by your " + armorName + "!");
						} else {
							if(eng->dice.coinToss()) {
								eng->player->getStatusEffectsHandler()->attemptAddEffect(new StatusSlowed(eng));
							} else {
								eng->player->getStatusEffectsHandler()->attemptAddEffect(new StatusCursed(eng));
							}
							restoreHP(999);
						}
					}
					eng->gameTime->letNextAct();
					return;
				}
			}
		}
	}

	if(attemptAttack(eng->player->pos))
		return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng))
		return;

	if(AI_stepToLairIfHasLosToLair::action(this, lairCell, eng) == true)
		return;

	vector<coord> path;
	AI_setPathToLairIfNoLosToLair::learn(this, &path, lairCell, eng);

	if(AI_stepPath::action(this, &path))
		return;

	eng->gameTime->letNextAct();
}

void Ghost::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_ghostClaw));
}

void Phantasm::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_phantasmSickle));
}

void Wraith::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_wraithClaw));
	eng->spellHandler->addAllCommonSpellsForMonsters(spellsKnown);
}

