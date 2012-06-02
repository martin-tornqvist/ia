#include "monsterIncludeFiles.h"

void Shadow::actorSpecificAct() {
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(AI_makeRoomForFriend::action(this, eng))
		return;

	if(eng->dice(1, 100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng))
			return;

	if(attemptAttack(eng->player->pos))
		return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng))
		return;

	if(AI_stepToLairIfHasLosToLair::action(this, lairCell, eng) == true)
		return;

	vector<coord> path;
	AI_setPathToLairIfNoLosToLair::learn(this, &path, lairCell, eng);

	if(AI_stepPath::action(this, &path))
		return;

	eng->gameTime->letNextAct();
}

void Shadow::actorSpecific_spawnStartItems() {
    m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_shadowClaw));
}
