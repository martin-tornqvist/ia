#include "monsterIncludeFiles.h"

void MiGo::actorSpecificAct()
{
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(eng->dice(1,12) == 1)
		if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	if(eng->dice(1,100) < m_instanceDefinition.erraticMovement)
		if(AI_castRandomSpellIfAware::action(this, eng)) return;

	if(attemptAttack(eng->player->pos)) {
		m_statusEffectsHandler->attemptAddEffect(new StatusDisabledAttackRanged(2));
		return;
	}

	if(AI_castRandomSpellIfAware::action(this, eng)) return;

	if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	eng->gameTime->letNextAct();
}

void MiGo::actorSpecific_spawnStartItems()
{
	Item* item = eng->itemFactory->spawnItem(item_miGoElectricGun);
	m_inventory->putItemInIntrinsics(item);

   spellsKnown.push_back(new SpellTeleport);

	if(eng->dice.coinToss()) {
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	}
}

