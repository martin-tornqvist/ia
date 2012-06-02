#include "monsterIncludeFiles.h"


void Ooze::actorSpecificAct()
{
	AI_look_becomePlayerAware::learn(this, eng);

	if(AI_makeRoomForFriend::action(this, eng)) return;

	if(eng->dice(1,100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	if(attemptAttack(eng->player->pos)) return;

	vector<coord> path;
	AI_setPathToPlayerIfAware::learn(this, &path, eng);

	if(AI_stepPath::action(this, &path)) return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng)) return;

	if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	eng->gameTime->letNextAct();
}

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
