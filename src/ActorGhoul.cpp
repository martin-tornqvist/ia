#include "monsterIncludeFiles.h"

void Ghoul::actorSpecific_spawnStartItems() {
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_ghoulClaw));
}

void Mummy::actorSpecific_spawnStartItems() {
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_mummyMaul));

    for(int i = eng->dice(1,3); i > 0; i--) {
        spellsKnown.push_back( eng->spellHandler->getRandomSpellForMonsters() );
    }
}

void MummyUnique::actorSpecific_spawnStartItems() {
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_mummyMaul));
    eng->spellHandler->addAllCommonSpellsForMonsters(spellsKnown);
}

void DeepOne::actorSpecific_spawnStartItems() {
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_deepOneJavelinAttack));
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_deepOneSpearAttack));
}
