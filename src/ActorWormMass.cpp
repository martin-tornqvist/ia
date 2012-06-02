#include "monsterIncludeFiles.h"

#include "ActorFactory.h"

void WormMass::actorSpecificAct() {
	AI_look_becomePlayerAware::learn(this, eng);

	//Multiply
	if(deadState == actorDeadState_alive) {
      if(playerAwarenessCounter > 0) {
         if(eng->dice(1, 100) < chanceToSpawnNew) {

            bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
            eng->mapTests->makeMoveBlockerArray(this, blockers);

            for(int dx = -1; dx <= 1; dx++) {
               for(int dy = -1; dy <= 1; dy++) {
                  if(blockers[pos.x + dx][pos.y + dy] == false) {
                     WormMass* const spawn = dynamic_cast<WormMass*> (eng->actorFactory->spawnActor(m_instanceDefinition.devName, pos + coord(dx,
                           dy)));
                     chanceToSpawnNew -= 5;
                     spawn->chanceToSpawnNew = chanceToSpawnNew;
                     eng->gameTime->letNextAct();
                     return;
                  }
               }
            }
         }
      }
	}

	if(AI_makeRoomForFriend::action(this, eng))
		return;

	if(attemptAttack(eng->player->pos))
		return;

	if(eng->dice(1, 100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng))
			return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng))
		return;

	eng->gameTime->letNextAct();
}

void WormMass::actorSpecific_spawnStartItems() {
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_wormMassBite));
}

