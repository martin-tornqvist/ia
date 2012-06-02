#include "monsterIncludeFiles.h"

void FireHound::actorSpecificAct()
{
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(AI_makeRoomForFriend::action(this, eng)) return;

	if(eng->dice(1,100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	if(attemptAttack(eng->player->pos)) {
		m_statusEffectsHandler->attemptAddEffect(new StatusDisabledAttackRanged(5));
		return;
	}

	vector<coord> path;
	AI_setPathToPlayerIfAware::learn(this, &path, eng);

	if(AI_handleClosedBlockingDoor::action(this, &path, eng) == true) return;

	if(AI_stepPath::action(this, &path)) return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng)) return;

	if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	eng->gameTime->letNextAct();
}

void FireHound::actorSpecific_spawnStartItems()
{
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundFireBreath));
	m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundBite));
}

