#include "monsterIncludeFiles.h"

void Rat::actorSpecificAct() {
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

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

void Rat::actorSpecific_spawnStartItems() {
	Item* item = NULL;

	if(eng->dice(1, 100) < 20) {
		item = eng->itemFactory->spawnItem(item_ratBiteDiseased);
	} else {
		item = eng->itemFactory->spawnItem(item_ratBite);
	}
	m_inventory->putItemInIntrinsics(item);
}

void RatThing::actorSpecific_spawnStartItems() {
	Item* item = eng->itemFactory->spawnItem(item_ratThingBite);
	m_inventory->putItemInIntrinsics(item);
}

void BrownJenkin::actorSpecificAct() {
	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(attemptAttack(eng->player->pos))
		return;

	if(eng->dice(1, 100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng))
			return;

	vector<coord> path;
	AI_setPathToPlayerIfAware::learn(this, &path, eng);

	if(AI_stepPath::action(this, &path))
		return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng))
		return;

	if(AI_moveToRandomAdjacentCell::action(this, eng))
		return;

	eng->gameTime->letNextAct();
}
