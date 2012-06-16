#include "monsterIncludeFiles.h"

#include "ActorFactory.h"

bool WormMass::actorSpecificAct() {
	if(deadState == actorDeadState_alive) {
		if(playerAwarenessCounter > 0) {
			if(eng->dice(1, 100) < chanceToSpawnNew) {

				bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
				eng->mapTests->makeMoveBlockerArray(this, blockers);

				for(int dx = -1; dx <= 1; dx++) {
					for(int dy = -1; dy <= 1; dy++) {
						if(blockers[pos.x + dx][pos.y + dy] == false) {
							WormMass* const spawn =
							   dynamic_cast<WormMass*>(eng->actorFactory->spawnActor(m_instanceDefinition.devName, pos + coord(dx, dy)));
							chanceToSpawnNew -= 5;
							spawn->chanceToSpawnNew = chanceToSpawnNew;
							eng->gameTime->letNextAct();
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

void WormMass::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_wormMassBite));
}

