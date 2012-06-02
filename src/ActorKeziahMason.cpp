#include "monsterIncludeFiles.h"

#include "Log.h"
#include "ActorFactory.h"
#include "Explosion.h"

void KeziahMason::actorSpecificAct() {
	if(deadState == actorDeadState_alive) {
		if(playerAwarenessCounter > 0) {
			if(hasSummonedJenkin == false) {

				bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
				eng->mapTests->makeVisionBlockerArray(blockers);

				if(checkIfSeeActor(*(eng->player), blockers)) {

					eng->mapTests->makeMoveBlockerArray(this, blockers);

					vector<coord> line = eng->mapTests->getLine(pos.x, pos.y, eng->player->pos.x, eng->player->pos.y, true, 9999);

					for(unsigned int i = 0; i < line.size(); i++) {
						const coord c = line.at(i);
						if(blockers[c.x][c.y] == false) {

							eng->log->addMessage(m_instanceDefinition.spellCastMessage);
							Monster* jenkin = dynamic_cast<Monster*> (eng->actorFactory->spawnActor(actor_brownJenkin, c));
							eng->explosionMaker->runSmokeExplosion(c);
							eng->renderer->drawMapAndInterface();
							hasSummonedJenkin = true;
							jenkin->playerAwarenessCounter = 999;

							return;
						}
					}
				}
			}
		}
	}

	AI_look_becomePlayerAware::learn(this, eng);

	AI_listen_becomePlayerAware::learn(this, soundsHeard);

	if(eng->dice(1, 100) < m_instanceDefinition.erraticMovement)
		if(AI_moveToRandomAdjacentCell::action(this, eng))
			return;

	if(AI_castRandomSpellIfAware::action(this, eng))
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

void KeziahMason::actorSpecific_spawnStartItems() {
	eng->spellHandler->addAllCommonSpellsForMonsters(spellsKnown);
}

