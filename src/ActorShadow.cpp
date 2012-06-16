#include "monsterIncludeFiles.h"

void Shadow::actorSpecific_spawnStartItems() {
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_shadowClaw));
}
