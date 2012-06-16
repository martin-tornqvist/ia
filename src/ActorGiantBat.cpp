#include "monsterIncludeFiles.h"

void GiantBat::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_giantBatBite));
}

void Byakhee::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_byakheeClaw));
}

void HuntingHorror::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_huntingHorrorBite));
}
