#include "monsterIncludeFiles.h"

void FireHound::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundFireBreath));
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundBite));
}
