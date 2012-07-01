#include "ItemScroll.h"

#include "ItemData.h"
#include "Engine.h"
#include "StatusEffects.h"
#include "Log.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "DungeonClimb.h"
#include "FeatureTrap.h"
#include "Postmortem.h"
#include "Query.h"

//void scrollOfSummoning::specificRead(const bool FROM_MEMORY, Engine* const engine) {
//	engine->mapTests->makeFreeCellsVector(LandscBlockData(true, true, false, false, false), actorsBlock_all, true, trapsBlock_never,
//			false, NULL);
//	const unsigned int NR_ELEMENTS = engine->mapTests->freeCellsVector.size();
//	const unsigned int ELEMENT = engine->dice(1, NR_ELEMENTS) - 1;
//	const coord CELL = engine->mapTests->freeCellsVector.at(ELEMENT);
//
//	engine->actorFactory->spawnActor(actor_giantBat, CELL);
//
//	const bool PLAYER_SEE_CELL = engine->map->playerVision[CELL.x][CELL.y] == true;
//	if(PLAYER_SEE_CELL == true) {
//		engine->log->addMessage("You see a creature appear!");
//	} else {
//		engine->log->addMessage("A creature appears somewhere...");
//	}
//	setRealDefinitionNames(engine, false);
//}
//
//void scrollOfGreaterSummoning::specificRead(const bool FROM_MEMORY, Engine* const engine) {
//	engine->mapTests->makeFreeCellsVector(LandscBlockData(true, true, false, false, false), actorsBlock_all, true, trapsBlock_never,
//			false, NULL);
//	const unsigned int NR_ELEMENTS = engine->mapTests->freeCellsVector.size();
//	const unsigned int ELEMENT = engine->dice(1, NR_ELEMENTS) - 1;
//	const coord CELL = engine->mapTests->freeCellsVector.at(ELEMENT);
//
//	engine->actorFactory->spawnActor(actor_fireHound, CELL);
//
//	const bool PLAYER_SEE_CELL = engine->map->playerVision[CELL.x][CELL.y] == true;
//	if(PLAYER_SEE_CELL == true) {
//		engine->log->addMessage("You see a powerful creature appear!");
//	} else {
//		engine->log->addMessage("A powerful creature appears somewhere...");
//	}
//	setRealDefinitionNames(engine, false);
//}

void ScrollOfMayhem::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;

	const int NR_OF_SWEEPS = 5;

	engine->log->addMessage("Destruction rages around you!");

	for(int i = 0; i < NR_OF_SWEEPS; i++) {
		for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
			for(int x = 1; x < MAP_X_CELLS - 1; x++) {
				const coord c(x,y);
				bool isAdjToWalkableCell = false;
				for(int dy = -1; dy <= 1; dy++) {
					for(int dx = -1; dx <= 1; dx++) {
						if(engine->map->featuresStatic[x + dx][y + dy]->isMoveTypePassable(moveType_walk)) {
							isAdjToWalkableCell = true;
						}
					}
				}
				if(isAdjToWalkableCell) {
					const int CHANCE_TO_DESTROY = 10;
					if(engine->dice(1,100) < CHANCE_TO_DESTROY) {
						engine->map->switchToDestroyedFeatAt(c);
					}
				}
			}
		}
	}

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			if(engine->map->featuresStatic[x][y]->canHaveBlood()) {
				const int CHANCE_FOR_BLOOD = 20;
				if(engine->dice(1,100) < CHANCE_FOR_BLOOD) {
					engine->map->featuresStatic[x][y]->setHasBlood(true);
				}
			}
		}
	}

	for(unsigned int i = 0; i < engine->gameTime->getLoopSize(); i++) {
		Actor* actor = engine->gameTime->getActorAt(i);
		if(actor != engine->player) {
			if(engine->player->checkIfSeeActor(*actor, NULL)) {
				actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBurning(engine));
			}
		}
	}

	setRealDefinitionNames(engine, false);
}

void ScrollOfPestilence::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;

	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	engine->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

	const int RADI = 5;
	const int x0 = max(0, engine->player->pos.x - RADI);
	const int y0 = max(0, engine->player->pos.y - RADI);
	const int x1 = min(MAP_X_CELLS - 1, engine->player->pos.x + RADI);
	const int y1 = min(MAP_Y_CELLS - 1, engine->player->pos.y + RADI);

	for(int x = x0; x <= x1; x++) {
		for(int y = y0; y <= y1; y++) {
			if(blockers[x][y] == false) {
				if(engine->dice(1, 3) == 1) {
					if(engine->dice.coinToss()) {
						if(engine->dice.coinToss()) {
							engine->actorFactory->spawnActor(actor_greenSpider, coord(x, y));
						} else {
							engine->actorFactory->spawnActor(actor_whiteSpider, coord(x, y));
						}
					} else {
						engine->actorFactory->spawnActor(actor_blackSpider, coord(x, y));
					}
				} else {
					engine->actorFactory->spawnActor(actor_rat, coord(x, y));
				}
			}
		}
	}

	engine->log->addMessage("Disgusting critters appear around you!");
	setRealDefinitionNames(engine, false);
}

void ScrollOfDeepDescent::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;
	if(engine->map->getDungeonLevel() <= LAST_CAVERN_LEVEL - 3) {
		engine->dungeonClimb->travelDown(1);
		engine->log->addMessage("You sink downwards!");
		setRealDefinitionNames(engine, false);
	} else {
		engine->log->addMessage("Nothing happens.");
	}
}

void ScrollOfTeleportation::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;
	engine->player->teleportToRandom();
	setRealDefinitionNames(engine, false);
}

void ScrollOfStatusOnAllVisibleMonsters::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;
	engine->player->getSpotedEnemies();
	vector<Actor*> actors = engine->player->spotedEnemies;

	if(actors.size() > 0) {

		setRealDefinitionNames(engine, false);

		for(unsigned int i = 0; i < actors.size(); i++) {
			if(actors.at(i) != engine->player) {
				StatusEffect* const effect = getStatusEffect(engine);
				actors.at(i)->getStatusEffectsHandler()->attemptAddEffect(effect);
			}
		}
	} else {
		failedToLearnRealName(engine);
	}
}

StatusEffect* ScrollOfConfuseEnemies::getStatusEffect(Engine* const engine) {
	return new StatusConfused(engine->dice(3, 6) + 6);
}

//StatusEffect* ScrollOfBlindEnemies::getStatusEffect(Engine* const engine) {
//    return new StatusBlind(engine->dice(3, 6) + 6);
//}

StatusEffect* ScrollOfParalyzeEnemies::getStatusEffect(Engine* const engine) {
	return new StatusParalyzed(engine->dice(1, 6) + 6);
}

StatusEffect* ScrollOfSlowEnemies::getStatusEffect(Engine* const engine) {
	return new StatusSlowed(engine->dice(3, 6) + 6);
}

void ScrollOfDetectItems::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;

	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			Item* item = engine->map->items[x][y];
			if(item != NULL) {
				engine->map->playerVision[x][y] = true;
				engine->map->explored[x][y] = true;
			}
		}
	}
	engine->renderer->drawMapAndInterface();
	engine->player->FOVupdate();
	engine->renderer->drawMapAndInterface();

	engine->log->addMessage("All items are revealed to you.");
	setRealDefinitionNames(engine, false);
}

void ScrollOfBlessing::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;

	engine->player->getStatusEffectsHandler()->attemptAddEffect(new StatusBlessed(engine));
	setRealDefinitionNames(engine, false);
}

void ScrollOfDetectTraps::specificRead(const bool FROM_MEMORY, Engine* const engine) {
	(void)FROM_MEMORY;
	bool somethingRevealed = false;

	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			FeatureStatic* const f = engine->map->featuresStatic[x][y];
			if(f->getId() == feature_trap) {
				Trap* const trap = dynamic_cast<Trap*>(f);
				trap->reveal(false);
				somethingRevealed = true;
			}
		}
	}

	if(somethingRevealed) {
		engine->log->addMessage("All traps are revealed to you.");
		setRealDefinitionNames(engine, false);
	} else {
		failedToLearnRealName(engine);
	}
}

/*void ScrollOfDoorObstruction::specificRead(const bool FROM_MEMORY, Actor* const  actor, Engine* const engine)
 {

 }*/

void ScrollNameHandler::setFalseScrollName(ItemDefinition* d) {
	const unsigned int NR_NAMES = m_falseNames.size();

	const unsigned int ELEMENT = static_cast<unsigned int>(eng->dice(1, NR_NAMES) - 1);

	const string TITLE = "\"" + m_falseNames.at(ELEMENT) + "\"";

	m_falseNames.erase(m_falseNames.begin() + ELEMENT);

	d->name.name = "Manuscript titled " + TITLE;
	d->name.name_plural = "Manuscripts titled " + TITLE;
	d->name.name_a = "a Manuscript titled " + TITLE;
}

void ScrollNameHandler::addSaveLines(vector<string>& lines) const {
	for(unsigned int i = 1; i < endOfItemDevNames; i++) {
		if(eng->itemData->itemDefinitions[i]->isReadable == true) {
			lines.push_back(eng->itemData->itemDefinitions[i]->name.name);
			lines.push_back(eng->itemData->itemDefinitions[i]->name.name_plural);
			lines.push_back(eng->itemData->itemDefinitions[i]->name.name_a);
		}
	}
}

void ScrollNameHandler::setParametersFromSaveLines(vector<string>& lines) {
	for(unsigned int i = 1; i < endOfItemDevNames; i++) {
		if(eng->itemData->itemDefinitions[i]->isReadable == true) {
			eng->itemData->itemDefinitions[i]->name.name = lines.front();
			lines.erase(lines.begin());
			eng->itemData->itemDefinitions[i]->name.name_plural = lines.front();
			lines.erase(lines.begin());
			eng->itemData->itemDefinitions[i]->name.name_a = lines.front();
			lines.erase(lines.begin());
		}
	}
}

//ItemActivateReturn_t Scroll::study(Engine* const engine) {
//	engine->renderer->drawMapAndInterface();
//	engine->log->addMessage("You study the manuscript...");
//
//	ItemActivateReturn_t returnData = itemActivate_keep;
//
//	const AbilityRollResult_t rollResult = engine->abilityRoll->roll(getChanceToLearnOrCastFromMemory(false, engine));
//
//	int chanceToDestroy = 35;
//
//	if(rollResult >= successSmall) {
//		if(m_archetypeDefinition->isIdentified == false) {
//			setRealDefinitionNames(engine, false);
//			engine->log->addMessage("It is " + m_archetypeDefinition->name.name_a + ".", clrWhite, true);
//		} else {
//			m_instanceDefinition.isScrollLearned = true;
//			m_archetypeDefinition->isScrollLearned = true;
//			engine->log->addMessage("The deeper meaning of this text becomes clear to you...");
//			chanceToDestroy = 100;
//			engine->player->shock(shockValue_heavy, 0);
//			engine->player->shock(shockValue_heavy, 0);
//		}
//	} else {
//		if(m_archetypeDefinition->isIdentified == true) {
//			engine->log->addMessage("You fail to comprehend the essence of it.");
//		} else {
//			engine->log->addMessage("You fail to decipher it.");
//		}
//	}
//
//	if(rollResult < successBig) {
//		//Insert bad stuff ***
//		//logMessagesAddedBetweenRefer = true:
//	}
//
//	if(engine->dice(1, 100) < chanceToDestroy) {
//		returnData = itemActivate_destroyed;
//		if(m_archetypeDefinition->isScrollLearned == true) {
//			engine->log->addMessage("The manuscript suddenly crumbles, but you retain it in your memory.");
//		} else {
//			engine->log->addMessage("The manuscript suddenly crumbles.");
//		}
//	}
//
//	engine->gameTime->letNextAct();
//
//	return returnData;
//}

int Scroll::getChanceToLearn(Engine* const engine) const {
	const int PLAYER_SKILL = engine->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_language, true);
	const int SCROLL_SKILL_FACTOR = m_archetypeDefinition->identifySkillFactor;

	return ((PLAYER_SKILL * SCROLL_SKILL_FACTOR) / 100) + 10;
}

int Scroll::getChanceToCastFromMemory(Engine* const engine) const {
	const int PLAYER_SKILL = engine->player->arcaneKnowledge;
	const int SCROLL_SKILL_FACTOR = m_archetypeDefinition->identifySkillFactor;

	return (PLAYER_SKILL * SCROLL_SKILL_FACTOR) / 100;
}

void Scroll::setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY) {
	if(m_archetypeDefinition->isIdentified == false) {
		const string REAL_TYPE_NAME = getRealTypeName();

		const string REAL_NAME = "Manuscript of " + REAL_TYPE_NAME;
		const string REAL_NAME_PLURAL = "Manuscripts of " + REAL_TYPE_NAME;
		const string REAL_NAME_A = "a Manuscript of " + REAL_TYPE_NAME;

		m_instanceDefinition.name.name = REAL_NAME;
		m_instanceDefinition.name.name_plural = REAL_NAME_PLURAL;
		m_instanceDefinition.name.name_a = REAL_NAME_A;

		m_archetypeDefinition->name.name = REAL_NAME;
		m_archetypeDefinition->name.name_plural = REAL_NAME_PLURAL;
		m_archetypeDefinition->name.name_a = REAL_NAME_A;

		//engine->log->addMessage("It was a " + REAL_NAME + ".");

		if(IS_SILENT_IDENTIFY == false) {
			engine->player->shock(shockValue_heavy, 0);
		}

		m_archetypeDefinition->isIdentified = true;
	}
}

bool Scroll::read(const bool IS_FROM_MEMORY, Engine* const engine) {
	engine->renderer->drawMapAndInterface();

	if(IS_FROM_MEMORY) {
		const AbilityRollResult_t rollResult = engine->abilityRoll->roll(getChanceToCastFromMemory(engine));
		if(rollResult >= successSmall) {
			engine->log->addMessage("You cast " + getRealTypeName() + "...");
			specificRead(IS_FROM_MEMORY, engine);
		} else {
			engine->log->addMessage("You miscast it!");
			if(engine->dice.coinToss()) {
				engine->log->addMessage("You feel a sharp pain as if from a surge of electricity.", clrMessageBad);
				engine->postmortem->setCauseOfDeath("Miscast a spell");
				engine->player->hit(engine->dice(1, 6), damageType_direct);
			}
		}
		engine->player->shock(shockValue_heavy, 0);
		engine->gameTime->letNextAct();
		return true;
	} else {
		if(m_archetypeDefinition->isIdentified) {

			specificRead(IS_FROM_MEMORY, engine);

			if(m_archetypeDefinition->isScrollLearned == false) {
				const AbilityRollResult_t identifyRollResult = engine->abilityRoll->roll(getChanceToLearn(engine));
				if(identifyRollResult >= successSmall) {
					engine->log->addMessage("You learn to cast this incantation by heart!");
					m_instanceDefinition.isScrollLearned = true;
					m_archetypeDefinition->isScrollLearned = true;
				}
			}
			engine->player->shock(shockValue_heavy, 0);
			engine->gameTime->letNextAct();
			return true;

		} else {
			const AbilityRollResult_t identifyRollResult = engine->abilityRoll->roll(getChanceToLearn(engine));
			if(identifyRollResult >= successSmall) {
				setRealDefinitionNames(engine, false);

				if(engine->player->deadState == actorDeadState_alive) {
					engine->log->addMessage("You identify it as " + m_instanceDefinition.name.name_a + "!");
               engine->renderer->flip();
					engine->log->addMessage("Perform the incantation (y/n)?", clrWhiteHigh);
					engine->renderer->flip();
					if(engine->query->yesOrNo()) {
						specificRead(IS_FROM_MEMORY, engine);
						engine->player->shock(shockValue_heavy, 0);
						engine->gameTime->letNextAct();
						return true;
					} else {
						engine->log->clearLog();
						engine->renderer->drawMapAndInterface();
						engine->gameTime->letNextAct();
						return false;
					}
				}
			} else {
				engine->log->addMessage("You recite forbidden incantations...");
				specificRead(IS_FROM_MEMORY, engine);
				engine->player->shock(shockValue_heavy, 0);
				engine->gameTime->letNextAct();
				return true;
			}
		}
	}
	return true;
}

void Scroll::failedToLearnRealName(Engine* const engine, const string overrideFailString) {
	if(m_archetypeDefinition->isIdentified == false) {
		if(overrideFailString != "") {
			engine->log->addMessage(overrideFailString);
		} else {
			engine->log->addMessage("Was that supposed to do something?");
		}
	}
}

