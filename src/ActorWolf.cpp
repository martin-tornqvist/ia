#include "monsterIncludeFiles.h"

void Wolf::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_wolfBite);
	m_inventory->putItemInIntrinsics(item);
}
