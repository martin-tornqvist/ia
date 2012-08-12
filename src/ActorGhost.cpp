#include "monsterIncludeFiles.h"

#include "Log.h"

bool Ghost::actorSpecificAct() {
	if(deadState == actorDeadState_alive) {
		if(playerAwarenessCounter > 0) {
			if(eng->mapTests->isCellsNeighbours(pos, eng->player->pos, false)) {
				if(eng->dice(1, 100) < 35) {

					bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
					eng->mapTests->makeVisionBlockerArray(blockers);
					const bool PLAYER_SEES_ME = eng->player->checkIfSeeActor(*this, blockers);
					const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
					eng->log->addMessage(refer + " reaches for me... ");
					const AbilityRollResult_t rollResult = eng->abilityRoll->roll(
					      eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_dodge, true));
					const bool PLAYER_DODGES = rollResult >= successSmall;
					if(PLAYER_DODGES) {
						eng->log->addMessage("I dodge!", clrMessageGood);
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
							eng->log->addMessage("The touch is deflected by my " + armorName + "!");
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
					return true;
				}
			}
		}
	}
	return false;
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

