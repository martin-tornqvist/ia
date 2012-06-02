#include "monsterIncludeFiles.h"

#include "Explosion.h"

void FireVampire::actorSpecificAct()
{
//	if(eng->mapTests->isCellsNeighbours(pos, eng->player->pos) == true) {
//		if(eng->dice(1,100) < 75) {
//			die(true, false, false);
//			eng->gameTime->letNextAct();
//			return;
//		}
//	}

	if(eng->dice(1,100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	if(attemptAttack(eng->player->pos))
		return;

	AI_look_becomePlayerAware::learn(this, eng);

	vector<coord> path;
	AI_setPathToPlayerIfAware::learn(this, &path, eng);

	if(AI_stepPath::action(this, &path)) return;

	if(AI_look_moveTowardsTargetIfVision::action(this, eng)) return;

	if(AI_moveToRandomAdjacentCell::action(this, eng)) return;

	eng->gameTime->letNextAct();
}

void FireVampire::monsterDeath()
{
	eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng), true, clrYellow);
}

void FireVampire::actorSpecific_spawnStartItems()
{
   m_inventory->putItemInIntrinsics(eng->itemFactory->spawnItem(item_fireVampireTouch));
}
