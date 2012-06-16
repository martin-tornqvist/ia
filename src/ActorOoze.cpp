#include "monsterIncludeFiles.h"

void OozeGray::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_oozeGraySpewPus));
}

void OozeClear::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_oozeClearSpewPus));
}

void OozePutrid::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_oozePutridSpewPus));
}
