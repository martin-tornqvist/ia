#include "monsterIncludeFiles.h"


void GiantBat::actorSpecificAct()
{
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(AI_makeRoomForFriend::action(this, eng)) return;

	if(eng->dice(1,100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	if(attemptAttack(eng->player->pos)) return;

	vector<coord> path;
	AI_setPathToPlayerIfAware::learn(this, &path, eng);

	if(AI_handleClosedBlockingDoor::action(this, &path, eng) == true) return;

	if(AI_stepPath::action(this, &path)) return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng)) return;

	if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	eng->gameTime->letNextAct();
}

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
