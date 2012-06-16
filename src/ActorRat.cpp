#include "monsterIncludeFiles.h"

void Rat::actorSpecific_spawnStartItems() {
	Item* item = NULL;
	if(eng->dice(1, 100) < 20) {
		item = eng->itemFactory->spawnItem(item_ratBiteDiseased);
	} else {
		item = eng->itemFactory->spawnItem(item_ratBite);
	}
	m_inventory->putItemInIntrinsics(item);
}

void RatThing::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_ratThingBite));
}
