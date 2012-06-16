#include "monsterIncludeFiles.h"

#include "Explosion.h"

void FireVampire::monsterDeath()
{
	eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng), true, clrYellow);
}

void FireVampire::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_fireVampireTouch));
}
