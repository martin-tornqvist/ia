#include "monsterIncludeFiles.h"

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

