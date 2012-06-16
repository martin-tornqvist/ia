#include "monsterIncludeFiles.h"

void CultistShotgun::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_sawedOff);
	m_inventory->putItemInSlot(slot_wielded, item, true);

	item = eng->itemFactory->spawnItem(item_shotgunShell);
	item->numberOfItems = 10;
	m_inventory->putItemInGeneral(item);

	if(eng->dice(1, 100) < 33) {
		m_inventory->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
	}

	if(eng->dice(1, 100) < 10) {
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	}
}

void CultistMachineGun::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_machineGun);
	m_inventory->putItemInSlot(slot_wielded, item, true);

	if(eng->dice(1, 100) < 33) {
		m_inventory->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
	}

	if(eng->dice(1, 100) < 10) {
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	}
}

void CultistTeslaCannon::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_teslaCanon);
	m_inventory->putItemInSlot(slot_wielded, item, true);

	item = eng->itemFactory->spawnItem(item_teslaCanister);
	item->numberOfItems = 4;
	m_inventory->putItemInGeneral(item);

	if(eng->dice(1, 100) < 33) {
		m_inventory->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
	}

	if(eng->dice(1, 100) < 10) {
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	}
}

void CultistSpikeGun::actorSpecific_spawnStartItems() {
	m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_spikeGun), true);
	Item* item = eng->itemFactory->spawnItem(item_ironSpike);
	item->numberOfItems = 8 + eng->dice(1, 8);
	m_inventory->putItemInGeneral(item);
}

void CultistPistol::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_pistol);
	m_inventory->putItemInSlot(slot_wielded, item, true);

	if(eng->dice(1, 100) < 38) {
		item = eng->itemFactory->spawnItem(item_pistolClip);
		m_inventory->putItemInGeneral(item);
	}

	if(eng->dice(1, 100) < 33) {
		m_inventory->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
	}

	if(eng->dice(1, 100) < 10) {
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	}
}

void CultistPriest::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_dagger);
	m_inventory->putItemInSlot(slot_wielded, item, true);

	m_inventory->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
	m_inventory->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));

	spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());

	if(eng->dice(1, 100) < 33) {
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
		spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
	}
}
