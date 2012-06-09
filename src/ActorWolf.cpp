#include "monsterIncludeFiles.h"

void Wolf::actorSpecificAct() {
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(AI_makeRoomForFriend::action(this, eng))
		return;

	if(eng->dice(1, 100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng))
			return;

	if(attemptAttack(eng->player->pos)) {
		m_statusEffectsHandler->attemptAddEffect(new StatusWaiting(2));
		return;
	}

	if(AI_look_moveTowardsTargetIfVision::action(this, eng))
		return;

	vector<coord> path;

   AI_setPathToLeaderIfNoLosToleader::learn(this, &path, eng);
   if(AI_stepPath::action(this, &path))
		return;

	AI_setPathToPlayerIfAware::learn(this, &path, eng);
	if(AI_stepPath::action(this, &path))
		return;

	if(AI_moveToRandomAdjacentCell::action(this, eng))
		return;

	eng->gameTime->letNextAct();
}

void Wolf::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_wolfBite);
	m_inventory->putItemInIntrinsics(item);
}

