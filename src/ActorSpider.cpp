#include "monsterIncludeFiles.h"


bool Spider::actorSpecificAct() {
   return false;
}


void GreenSpider::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_greenSpiderBite));
}

void BlackSpider::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_blackSpiderBite));
}


void WhiteSpider::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_whiteSpiderBite));
}

void LengSpider::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_lengSpiderBite));
}
