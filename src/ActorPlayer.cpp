#include "ActorPlayer.h"

#include "Engine.h"

#include "ItemWeapon.h"
#include "FeatureTrap.h"
#include "PlayerCreateCharacter.h"
#include "Log.h"
#include "Interface.h"
#include "Popup.h"
#include "Postmortem.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Explosion.h"
#include "ActorMonster.h"
#include "FeatureDoor.h"
#include "Query.h"
#include "Attack.h"
#include "PlayerVisualMemory.h"
#include "Fov.h"
#include "ItemFactory.h"
#include "ActorFactory.h"

Player::Player() :
	firstAidTurnsLeft(-1), waitTurnsLeft(-1), insanityLong(0), insanityShort(0), insanityShortTemp(0), dynamiteFuseTurns(-1), molotovFuseTurns(-1),
	flareFuseTurns(-1), target(NULL) {
}

void Player::actorSpecific_spawnStartItems() {
	m_instanceDefinition.abilityValues.reset();

	for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
		insanityPhobias[i] = false;
	}
	for(unsigned int i = 0; i < endOfInsanityCompulsions; i++) {
		insanityCompulsions[i] = false;
	}

//	const PlayerBackgrounds_t background = eng->playerBackgroundsHandler->getPlayerBackground();
//	switch(background) {
//	case playerBackground_soldier: {
//		m_inventory->putItemInSlot(slot_armorBody, eng->itemFactory->spawnItem(item_armorFlackJacket), true, true);
//
//		ItemDevNames_t weaponId;
//		int wpnNr = eng->dice(1,4);
//		switch(wpnNr) {
//		case 1: weaponId = item_hatchet; break;
//		case 2: weaponId = item_machete; break;
//		case 3: weaponId = item_club; break;
//		case 4: weaponId = item_axe; break;
//		}
//		m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);
//
//		wpnNr = eng->dice(1,3);
//		switch(wpnNr) {
//		case 1: {
//			m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_machineGun), true, true);
//			m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_drumOfBullets));
//		}
//		break;
//		case 2: {
//			m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_sawedOff), true, true);
//			Item* item = eng->itemFactory->spawnItem(item_shotgunShell);
//			item->numberOfItems = eng->dice.getInRange(12, 16);
//			m_inventory->putItemInGeneral(item);
//		}
//		break;
//		case 3: {
//			m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_pumpShotgun), true, true);
//			Item* item = eng->itemFactory->spawnItem(item_shotgunShell);
//			item->numberOfItems = eng->dice.getInRange(6, 10);
//			m_inventory->putItemInGeneral(item);
//		}
//		break;
//		}
//
//		m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
//		m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
//
//		Item* item = eng->itemFactory->spawnItem(item_dynamite);
//		item->numberOfItems = eng->dice.getInRange(2, 3);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_molotov);
//		item->numberOfItems = eng->dice.getInRange(2, 3);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_ironSpike);
//		item->numberOfItems = eng->dice.getInRange(5, 8);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_flare);
//		item->numberOfItems = eng->dice.getInRange(3, 5);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_throwingKnife);
//		item->numberOfItems = eng->dice.getInRange(8, 12);
//		m_inventory->putItemInSlot(slot_missiles, item, true, true);
//
//		eng->playerBonusHandler->increaseBonus(ability_accuracyRanged);
//		eng->playerBonusHandler->increaseBonus(ability_accuracyRanged);
//		eng->playerBonusHandler->increaseBonus(ability_weaponHandling);
//		eng->playerBonusHandler->increaseBonus(ability_mobility);
//		eng->playerBonusHandler->increaseBonus(ability_resistStatusBodyAndSense);
//
//		m_instanceDefinition.HP_max = 18;
//		m_instanceDefinition.HP = m_instanceDefinition.HP_max;
//	}
//	break;
//
//	case playerBackground_occultScholar: {
//		for(int i = 0; i < 2; i++) {
//			bool done = false;
//			while(done == false) {
//				Item* item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
//				ItemDefinition& archDef = item->getArchetypeDefinition();
//				if(archDef.isScrollLearned == false && archDef.isScrollLearnable) {
//				   item->setRealDefinitionNames(eng, true);
//					done = archDef.isScrollLearned = true;
//				}
//            delete item;
//				item = NULL;
//			}
//		}
//
//		ItemDevNames_t weaponId;
//		int wpnNr = eng->dice(1,2);
//		switch(wpnNr) {
//		case 1: weaponId = item_dagger; break;
//		case 2: weaponId = item_hatchet; break;
//		}
//		m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);
//
//		m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
//		m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
//
//		Item* item = eng->itemFactory->spawnItem(item_dynamite);
//		item->numberOfItems = eng->dice.getInRange(1, 2);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_molotov);
//		item->numberOfItems = eng->dice.getInRange(1, 2);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_ironSpike);
//		item->numberOfItems = eng->dice.getInRange(5, 8);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_flare);
//		item->numberOfItems = eng->dice.getInRange(3, 5);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
//		item->numberOfItems = eng->dice.getInRange(2,3);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
//		item->numberOfItems = eng->dice.getInRange(2,3);
//		item->setRealDefinitionNames(eng, true);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
//		item->numberOfItems = eng->dice.getInRange(2,3);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
//		item->numberOfItems = eng->dice.getInRange(2,3);
//		item->setRealDefinitionNames(eng, true);
//		m_inventory->putItemInGeneral(item);
//
//		eng->playerBonusHandler->increaseBonus(ability_loreArcana);
//		eng->playerBonusHandler->increaseBonus(ability_loreLanguage);
//		eng->playerBonusHandler->increaseBonus(ability_loreLanguage);
//		eng->playerBonusHandler->increaseBonus(ability_resistStatusMindAndShock);
//		eng->playerBonusHandler->increaseBonus(ability_resistStatusMindAndShock);
//
//		m_instanceDefinition.HP_max = 14;
//		m_instanceDefinition.HP = m_instanceDefinition.HP_max;
//	}
//	break;
//
//	case playerBackground_tombRaider: {
//		m_inventory->putItemInSlot(slot_armorBody, eng->itemFactory->spawnItem(item_armorLeatherJacket), true, true);
//
//		Item* item = eng->itemFactory->spawnItem(item_dagger);
//		dynamic_cast<Weapon*>(item)->setMeleePlus(2);
//		m_inventory->putItemInSlot(slot_wielded, item, true, true);
//
//		m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
//		m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
//
//		item = eng->itemFactory->spawnItem(item_dynamite);
//		item->numberOfItems = eng->dice.getInRange(2, 3);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_molotov);
//		item->numberOfItems = eng->dice.getInRange(2, 3);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_ironSpike);
//		item->numberOfItems = eng->dice.getInRange(5, 8);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_flare);
//		item->numberOfItems = eng->dice.getInRange(3, 5);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnItem(item_throwingKnife);
//		item->numberOfItems = eng->dice.getInRange(8, 12);
//		m_inventory->putItemInSlot(slot_missiles, item, true, true);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
//		item->numberOfItems = eng->dice.getInRange(1,2);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
//		item->numberOfItems = eng->dice.getInRange(1,2);
//		item->setRealDefinitionNames(eng, true);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
//		item->numberOfItems = eng->dice.getInRange(1,2);
//		m_inventory->putItemInGeneral(item);
//
//		item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
//		item->numberOfItems = eng->dice.getInRange(1,2);
//		item->setRealDefinitionNames(eng, true);
//		m_inventory->putItemInGeneral(item);
//
//		eng->playerBonusHandler->increaseBonus(ability_searching);
//		eng->playerBonusHandler->increaseBonus(ability_sneaking);
//		eng->playerBonusHandler->increaseBonus(ability_sneaking);
//		eng->playerBonusHandler->increaseBonus(ability_backstabbing);
//		eng->playerBonusHandler->increaseBonus(ability_dodge);
//
//		m_instanceDefinition.HP_max = 16;
//		m_instanceDefinition.HP = m_instanceDefinition.HP_max;
//	}
//	break;
//
//	default: {} break;
//	}

	int NR_OF_CARTRIDGES = eng->dice.getInRange(1, 3);
	int NR_OF_DYNAMITE = eng->dice.getInRange(2, 4);
	int NR_OF_MOLOTOV = eng->dice.getInRange(2, 4);
	int NR_OF_FLARES = eng->dice.getInRange(3, 5);
	int NR_OF_THROWING_KNIVES = eng->dice.getInRange(7, 12);
	int NR_OF_SPIKES = eng->dice.getInRange(3, 4);

	const int NR_OF_POSSIBLE_WEAPONS = 6;

	const int WEAPON_CHOICE = eng->dice.getInRange(1, NR_OF_POSSIBLE_WEAPONS);
	ItemDevNames_t weaponId = item_dagger;
	switch(WEAPON_CHOICE) {
	case 1:
		weaponId = item_dagger;
		break;
	case 2:
		weaponId = item_hatchet;
		break;
	case 3:
		weaponId = item_club;
		break;
	case 4:
		weaponId = item_hammer;
		break;
	case 5:
		weaponId = item_machete;
		break;
	case 6:
		weaponId = item_axe;
		break;

	default:
		weaponId = item_dagger;
		break;
	}

	m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);

	m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);

	for(int i = 0; i < NR_OF_CARTRIDGES; i++) {
		m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
	}

	Item* item = eng->itemFactory->spawnItem(item_dynamite);
	item->numberOfItems = NR_OF_DYNAMITE;
	m_inventory->putItemInGeneral(item);

	item = eng->itemFactory->spawnItem(item_molotov);
	item->numberOfItems = NR_OF_MOLOTOV;
	m_inventory->putItemInGeneral(item);

	item = eng->itemFactory->spawnItem(item_flare);
	item->numberOfItems = NR_OF_FLARES;
	m_inventory->putItemInGeneral(item);

	item = eng->itemFactory->spawnItem(item_throwingKnife);
	item->numberOfItems = NR_OF_THROWING_KNIVES;
	m_inventory->putItemInSlot(slot_missiles, item, true, true);

	item = eng->itemFactory->spawnItem(item_ironSpike);
	item->numberOfItems = NR_OF_SPIKES;
	m_inventory->putItemInGeneral(item);
}

void Player::addSaveLines(vector<string>& lines) const {
	const unsigned int NR_OF_STATUS_EFFECTS = m_statusEffectsHandler->effects.size();
	lines.push_back(intToString(NR_OF_STATUS_EFFECTS));
	for(unsigned int i = 0; i < NR_OF_STATUS_EFFECTS; i++) {
		lines.push_back(intToString(m_statusEffectsHandler->effects.at(i)->getEffectId()));
		lines.push_back(intToString(m_statusEffectsHandler->effects.at(i)->turnsLeft));
	}

	lines.push_back(intToString(insanityLong));
	lines.push_back(intToString(insanityShort));
	lines.push_back(intToString(m_instanceDefinition.HP));
	lines.push_back(intToString(m_instanceDefinition.HP_max));
	lines.push_back(intToString(pos.x));
	lines.push_back(intToString(pos.y));
	lines.push_back(intToString(dynamiteFuseTurns));
	lines.push_back(intToString(molotovFuseTurns));
	lines.push_back(intToString(flareFuseTurns));

	for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
		lines.push_back(insanityPhobias[i] == 0 ? "0" : "1");
	}
	for(unsigned int i = 0; i < endOfInsanityCompulsions; i++) {
		lines.push_back(insanityCompulsions[i] == 0 ? "0" : "1");
	}
}

void Player::actorSpecific_hit(const int DMG) {
	if(insanityCompulsions[insanityCompulsion_masochism] && DMG > 1) {
		insanityShort = max(0, insanityShort - 5);
	}

	//Hit aborts first aid
	if(firstAidTurnsLeft != -1) {
		firstAidTurnsLeft = -1;
		eng->log->addMessage("Your applying of first aid is disrupted.", clrWhite, false);
	}

	eng->renderer->drawMapAndInterface(true);
}

void Player::setParametersFromSaveLines(vector<string>& lines) {
	const unsigned int NR_OF_STATUS_EFFECTS = stringToInt(lines.front());
	lines.erase(lines.begin());
	for(unsigned int i = 0; i < NR_OF_STATUS_EFFECTS; i++) {
		const StatusEffects_t id = static_cast<StatusEffects_t>(stringToInt(lines.front()));
		lines.erase(lines.begin());
		const int TURNS = stringToInt(lines.front());
		lines.erase(lines.begin());
		m_statusEffectsHandler->attemptAddEffect(m_statusEffectsHandler->makeEffectFromId(id, TURNS), true, true);
	}

	insanityLong = stringToInt(lines.front());
	lines.erase(lines.begin());
	insanityShort = stringToInt(lines.front());
	lines.erase(lines.begin());
	m_instanceDefinition.HP = stringToInt(lines.front());
	lines.erase(lines.begin());
	m_instanceDefinition.HP_max = stringToInt(lines.front());
	lines.erase(lines.begin());
	pos.x = stringToInt(lines.front());
	lines.erase(lines.begin());
	pos.y = stringToInt(lines.front());
	lines.erase(lines.begin());
	dynamiteFuseTurns = stringToInt(lines.front());
	lines.erase(lines.begin());
	molotovFuseTurns = stringToInt(lines.front());
	lines.erase(lines.begin());
	flareFuseTurns = stringToInt(lines.front());
	lines.erase(lines.begin());

	for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
		insanityPhobias[i] = lines.front() == "0" ? false : true;
		lines.erase(lines.begin());
	}
	for(unsigned int i = 0; i < endOfInsanityCompulsions; i++) {
		insanityCompulsions[i] = lines.front() == "0" ? false : true;
		lines.erase(lines.begin());
	}
}

void Player::shock(const ShockValues_t shockValue, const int MODIFIER) {
	//const int ABILITY_PENALTY = 10;
	const int PLAYER_ABILITY = m_instanceDefinition.abilityValues.getAbilityValue(ability_resistStatusMindAndShock, true);
	const int ABILITY = max(5, PLAYER_ABILITY);
	if(eng->abilityRoll->roll(ABILITY) <= failSmall) {
		int baseInsIncr = 0;
		switch(shockValue) {
		case shockValue_none: {
			baseInsIncr = 0;
		}
		break;
		case shockValue_mild: {
			baseInsIncr = eng->dice(1, 2);
		}
		break;
		case shockValue_some: {
			baseInsIncr = eng->dice(1, 4);
		}
		break;
		case shockValue_heavy: {
			baseInsIncr = eng->dice(2, 4) + 4;
		}
		break;
		}
		insanityShort = min(100, insanityShort + max(0, baseInsIncr + MODIFIER));
	}
}

void Player::incrInsanityLong() {
	string popupMessage = "Insanity draws nearer... ";

	insanityLong += eng->dice(1, 4) + 2;

	insanityShort = max(0, insanityShort - 60);

	updateColor();
	eng->renderer->drawMapAndInterface();

	if(insanityLong >= 100) {
		popupMessage += "Your mind can no longer withstand what it has grasped. You are hopelessly lost.";
		eng->popup->showMessage(popupMessage);
		eng->postmortem->setCauseOfDeath("Insanity");
		die(true, false, false);
	} else {
		bool playerSeeShockingMonster = false;
		getSpotedEnemies();
		for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
			const ActorDefinition* const def = spotedEnemies.at(i)->getInstanceDefinition();
			if(def->shockValue > shockValue_none) {
				playerSeeShockingMonster = true;
			}
		}

		//When long term sanity decreases something happens (mostly bad mmkay)
		//(Reroll until something actually happens)
		for(unsigned int insAttemptCount = 0; insAttemptCount < 1000; insAttemptCount++) {
			const int ROLL = eng->dice(1, 8);
			switch(ROLL) {
			case 1: {
				if(playerSeeShockingMonster == true) {
					if(eng->dice.coinToss()) {
						popupMessage += "You let out a terrified shriek.";
					} else {
						popupMessage += "You scream in terror.";
					}
					eng->popup->showMessage(popupMessage);
					eng->soundEmitter->emitSound(Sound("", true, pos, 5, true));
					return;
				}
			}
			break;
			case 2: {
				popupMessage += "You find yourself babbling incoherently.";
				eng->popup->showMessage(popupMessage);
				for(int i = eng->dice(1, 3); i > 0; i--) {
					const string* const phrase = eng->phrases->getAggroPhraseFromPhraseSet(phraseSet_cultist);
					eng->log->addMessage("[" + getNameThe() + "]" + *phrase, clrYellow);
				}
				eng->soundEmitter->emitSound(Sound("", true, pos, 3, true));
				return;
			}
			break;
			case 3: {
				popupMessage += "You struggle to not fall into a stupor.";
				eng->popup->showMessage(popupMessage);
				m_statusEffectsHandler->attemptAddEffect(new StatusFainted(eng));
				return;
			}
			break;
			case 4: {
				popupMessage += "You laugh nervously.";
				eng->popup->showMessage(popupMessage);
				eng->soundEmitter->emitSound(Sound("", true, pos, 3, true));
				return;
			}
			break;
			case 5: {
				popupMessage += "Thanks to the mercy of the mind, some past experiences are forgotten (-5% xp).";
				eng->popup->showMessage(popupMessage);
				eng->dungeonMaster->playerLoseXpPercent(5);
				return;
			}
			break;
			case 6: {
				//There is a limit to the number of phobias you can have
				int phobiasActive = 0;
				for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
					if(insanityPhobias[i] == true) {
						phobiasActive++;
					}
				}
				if(phobiasActive < 2) {
					if(eng->dice(1, 2) == 1) {
						if(spotedEnemies.size() > 0) {
							const int MONSTER_ROLL = eng->dice(1, spotedEnemies.size()) - 1;
							if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isRat == true && insanityPhobias[insanityPhobia_rat] == false) {
								popupMessage += "You are afflicted by Murophobia. Rats suddenly seem terrifying.";
								eng->popup->showMessage(popupMessage);
								insanityPhobias[insanityPhobia_rat] = true;
								return;
							}
							if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isSpider == true && insanityPhobias[insanityPhobia_spider]
							      == false) {
								popupMessage += "You are afflicted by Arachnophobia. Spiders suddenly seem terrifying.";
								eng->popup->showMessage(popupMessage);
								insanityPhobias[insanityPhobia_spider] = true;
								return;
							}
							if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isCanine == true && insanityPhobias[insanityPhobia_dog]
							      == false) {
								popupMessage += "You are afflicted by Cynophobia. Dogs suddenly seem terrifying.";
								eng->popup->showMessage(popupMessage);
								insanityPhobias[insanityPhobia_dog] = true;
								return;
							}
							if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isUndead == true && insanityPhobias[insanityPhobia_undead]
							      == false) {
								popupMessage += "You are afflicted by Necrophobia. The undead suddenly seem much more terrifying.";
								eng->popup->showMessage(popupMessage);
								insanityPhobias[insanityPhobia_undead] = true;
								return;
							}
						}
					} else {
						if(eng->dice(1, 2) == 1) {
							if(isStandingInOpenPlace() == true) {
								if(insanityPhobias[insanityPhobia_openPlace] == false) {
									popupMessage += "You are afflicted by Agoraphobia. Open places suddenly seem terrifying.";
									eng->popup->showMessage(popupMessage);
									insanityPhobias[insanityPhobia_openPlace] = true;
									return;
								}
							} else {
								if(insanityPhobias[insanityPhobia_closedPlace] == false) {
									popupMessage += "You are afflicted by Claustrophobia. Confined places suddenly seem terrifying.";
									eng->popup->showMessage(popupMessage);
									insanityPhobias[insanityPhobia_closedPlace] = true;
									return;
								}
							}
						} else {
							if(eng->map->getDungeonLevel() >= 5) {
								if(insanityPhobias[insanityPhobia_deepPlaces] == false) {
									popupMessage += "You are afflicted by Bathophobia. It suddenly seems terrifying to delve deeper.";
									eng->popup->showMessage(popupMessage);
									insanityPhobias[insanityPhobia_deepPlaces] = true;
									return;
								}
							}
						}
					}
				}
			}
			break;
			case 7: {
				if(insanityLong > 50) {
					int compulsionsActive = 0;
					for(unsigned int i = 0; i < endOfInsanityCompulsions; i++) {
						if(insanityCompulsions[i] == true) {
							compulsionsActive++;
						}
					}
					if(compulsionsActive == 0) {
						const InsanityCompulsions_t compulsion = static_cast<InsanityCompulsions_t>(eng->dice.getInRange(0, endOfInsanityCompulsions - 1));
						switch(compulsion) {
						case insanityCompulsion_masochism: {
							popupMessage
							+= "To your alarm, you find yourself encouraged by the sensation of pain. Every time you are hurt, you find a little relief. However, your depraved mind decays faster over time now.";
							eng->popup->showMessage(popupMessage);
							insanityCompulsions[insanityCompulsion_masochism] = true;
							return;
						}
						break;
						case insanityCompulsion_sadism: {
							popupMessage
							+= "To your alarm, you find yourself encouraged by the pain you cause in others. For every life you take, you find a little relief. However, your depraved mind decays faster over time now.";
							eng->popup->showMessage(popupMessage);
							insanityCompulsions[insanityCompulsion_sadism] = true;
							return;
						}
						break;
						default: {
						}
						break;
						}
					}
				}
			}
			break;

			case 8: {
				popupMessage += "The shadows are closing in on you.";
				eng->popup->showMessage(popupMessage);

				bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
				eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

				int shadowsLeftToSpawn = eng->dice.getInRange(2, 3);
				while(shadowsLeftToSpawn > 0) {
					const int D_MAX = 3;
					for(int dx = -D_MAX; dx <= D_MAX; dx++) {
						for(int dy = -D_MAX; dy <= D_MAX; dy++) {
							if(dx <= -2 || dx >= 2 || dy <= -2 || dy >= 2) {
								if(blockers[pos.x + dx][pos.y + dy] == false) {
									if(eng->dice(1,100) < 10) {
										Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_shadow, pos + coord(dx, dy)));
										if(eng->dice.coinToss()) {
											monster->isSneaking = true;
										}
										shadowsLeftToSpawn--;
										if(shadowsLeftToSpawn <= 0) {
											dx = 999;
											dy = 999;
										}
									}
								}
							}
						}
					}
				}

				return;
			}

			default: {
			}
			break;
			}
		}
	}
}

bool Player::isStandingInOpenPlace() const {
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeMoveBlockerArrayFeaturesOnly(this, blockers);
	int nrBlockingCells = 0;
	for(int x = pos.x - 1; x <= pos.x + 1; x++) {
		for(int y = pos.y - 1; y <= pos.y + 1; y++) {
			if(blockers[x][y]) {
				nrBlockingCells++;
				if(nrBlockingCells >= 4) {
					return false;
				}
			}
		}
	}

	return true;
}

void Player::testPhobias() {
	const int ROLL = eng->dice(1, 100);
	//Phobia vs creature type?
	if(ROLL < 10) {
		for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
			if(spotedEnemies.at(0)->getInstanceDefinition()->isCanine == true && insanityPhobias[insanityPhobia_dog] == true) {
				eng->log->addMessage("You are plagued by your canine phobia!");
				m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
				return;
			}
			if(spotedEnemies.at(0)->getInstanceDefinition()->isRat == true && insanityPhobias[insanityPhobia_rat] == true) {
				eng->log->addMessage("You are plagued by your rat phobia!");
				m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
				return;
			}
			if(spotedEnemies.at(0)->getInstanceDefinition()->isUndead == true && insanityPhobias[insanityPhobia_undead] == true) {
				eng->log->addMessage("You are plagued by your phobia of the dead!");
				m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
				return;
			}
			if(spotedEnemies.at(0)->getInstanceDefinition()->isSpider == true && insanityPhobias[insanityPhobia_spider] == true) {
				eng->log->addMessage("You are plagued by your spider phobia!");
				m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
				return;
			}
		}
	}
	if(ROLL < 5) {
		if(isStandingInOpenPlace() == true) {
			if(insanityPhobias[insanityPhobia_openPlace] == true) {
				eng->log->addMessage("You are plagued by your phobia of open places!");
				m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
				return;
			}
		} else {
			if(insanityPhobias[insanityPhobia_closedPlace] == true) {
				eng->log->addMessage("You are plagued by your phobia of closed places!");
				m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
				return;
			}
		}
	}

	if(eng->map->featuresStatic[pos.x][pos.y]->getId() == feature_stairsDown && insanityPhobias[insanityPhobia_deepPlaces]) {
		eng->log->addMessage("You are plagued by your phobia of deep places!");
		m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(2, 6) + 6));
		return;
	}
}

void Player::updateColor() {
    SDL_Color& clr = m_instanceDefinition.color;

    if(deadState != actorDeadState_alive) {
        clr = clrRedLight;
        return;
    }

    const SDL_Color clrFromStatusEffect = m_statusEffectsHandler->getColor();
    if(clrFromStatusEffect.r != 0 || clrFromStatusEffect.g != 0 || clrFromStatusEffect.b != 0) {
        clr = clrFromStatusEffect;
        return;
    }

    if(dynamiteFuseTurns > 0 || molotovFuseTurns > 0 || flareFuseTurns > 0) {
        clr = clrYellow;
        return;
    }

    const SDL_Color& archetypeClr = m_archetypeDefinition->color;

    const int CUR_SHOCK = insanityShort + insanityShortTemp;

    if(CUR_SHOCK > 60) {
        const SDL_Color insaneClr = clrMagenta;

        const double fIns = static_cast<double>((CUR_SHOCK * 2) - 100) / 100;
        const double fArc = 1.0 - fIns;

        clr.r = (static_cast<double>(archetypeClr.r) * fArc) + (static_cast<double>(insaneClr.r) * fIns);
        clr.g = (static_cast<double>(archetypeClr.g) * fArc) + (static_cast<double>(insaneClr.g) * fIns);
        clr.b = (static_cast<double>(archetypeClr.b) * fArc) + (static_cast<double>(insaneClr.b) * fIns);
        return;
    }

    clr = archetypeClr;
}

void Player::act() {
	// Dynamite
	if(dynamiteFuseTurns > 0) {
		dynamiteFuseTurns--;
		if(dynamiteFuseTurns > 0) {
			string fuseMessage = "***F";
			for(int i = 0; i < dynamiteFuseTurns; i++) {
				fuseMessage += "Z";
			}
			fuseMessage += "***";
			eng->log->addMessage(fuseMessage, clrYellow);
		}
	}
	if(dynamiteFuseTurns == 0) {
		eng->log->addMessage("The dynamite explodes in your hands!");
		eng->explosionMaker->runExplosion(pos);
		updateColor();
		dynamiteFuseTurns = -1;
	}

	//Molotovs
	if(molotovFuseTurns > 0) {
		molotovFuseTurns--;
	}
	if(molotovFuseTurns == 0) {
		eng->log->addMessage("The Molotov Cocktail explodes in your hands!");
		updateColor();
		eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng));
		molotovFuseTurns = -1;
	}

	//Flare
	if(flareFuseTurns > 0) {
		flareFuseTurns--;
	}
	if(flareFuseTurns == 0) {
		eng->log->addMessage("The flare is extinguished.");
		updateColor();
		flareFuseTurns = -1;
	}

	getSpotedEnemies();

	//Any phobia that causes fear?
	testPhobias();

	//Lose short-term sanity from seen monsters?
	for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
		Monster* monster = dynamic_cast<Monster*>(spotedEnemies.at(i));
		const ActorDefinition* const def = monster->getInstanceDefinition();
		shock(def->shockValue, -(monster->shockDecrease));
		monster->shockDecrease++;
	}

	//Some short term sanity is lost every x turn
	const int TURN = eng->gameTime->getTurn();
	bool hasCompulsion = false;
	for(unsigned int i = 0; i < endOfInsanityCompulsions; i++) {
		if(insanityCompulsions[i] == true) {
			hasCompulsion = true;
		}
	}
	const int LOSE_N_TURN = hasCompulsion == true ? 12 : 17;
	if((TURN / LOSE_N_TURN) * LOSE_N_TURN == TURN && TURN > 1) {
		if(eng->dice(1, 1000) <= 3) {
			if(eng->dice.coinToss()) {
				eng->popup->showMessage("You have a bad feeling...");
			} else {
				eng->popup->showMessage("A chill runs down your spine...");
			}
			shock(shockValue_heavy, 0);
			shock(shockValue_heavy, 0);
			eng->renderer->drawMapAndInterface(true);
		} else {
			if(eng->map->getDungeonLevel() != 0) {
				shock(shockValue_mild, 0);
			}
		}
	}

	//Lose long-term sanity from high short-term sanity?
	if(insanityShort + insanityShortTemp >= 100) {
		/*Examples:
		 (divided by 10)
		 sanityShort		Risk (%)
		 --------------------
		 50				0
		 40				1
		 25				2.5 (2)
		 20				3
		 10				4
		 0				5

		 (divided by 5)
		 sanityShort		Risk (%)
		 --------------------
		 50				0
		 40				2
		 25				5
		 20				6
		 10				8
		 0				10
		 */

		incrInsanityLong();
		eng->gameTime->letNextAct();
		return;
	}

	const unsigned int LOOP_SIZE = eng->gameTime->getLoopSize();

	for(unsigned int i = 0; i < LOOP_SIZE; i++) {
		//If auto-traveling or applying first aid etc, messages may be printed for monsters that comes into view.
		//Only print monster-comes-into-view-messages if player is busy with something (first aid, auto travel etc).

		Actor* const actor = eng->gameTime->getActorAt(i);
		if(actor != this) {
			if(actor->deadState == actorDeadState_alive) {

				Monster* const monster = dynamic_cast<Monster*>(actor);
				const bool IS_MONSTER_SEEN = checkIfSeeActor(*actor, NULL);
				if(IS_MONSTER_SEEN) {
					if(monster->messageMonsterInViewPrinted == false) {

						if(firstAidTurnsLeft > 0 || waitTurnsLeft > 0) {
							eng->renderer->drawMapAndInterface();
							const string MONSTER_NAME = actor->getNameA();
							eng->log->addMessage(MONSTER_NAME + " comes into view.");
						}
						monster->messageMonsterInViewPrinted = true;
					}
				} else {
					monster->messageMonsterInViewPrinted = false;

					//Is the monster sneaking? Try to spot it
					if(eng->map->playerVision[monster->pos.x][monster->pos.y]) {
						if(monster->isSneaking) {
							const int PLAYER_SEARCH_SKILL = m_instanceDefinition.abilityValues.getAbilityValue(ability_searching, true);
							const AbilityRollResult_t rollResult = eng->abilityRoll->roll(PLAYER_SEARCH_SKILL);
							if(rollResult == successSmall) {
								eng->log->addMessage("You see something moving in the shadows.");
							}
							else if(rollResult > successSmall) {
								monster->isSneaking = false;
								FOVupdate();
								eng->renderer->drawMapAndInterface();
								eng->log->addMessage("You spot " + monster->getNameA() + "!");
							}
						}
					}
				}
			}
		}
	}

	if(firstAidTurnsLeft == -1) {
		if(m_statusEffectsHandler->allowSee()) {

			//Look for secret doors and traps
			for(int dx = -1; dx <= 1; dx++) {
				for(int dy = -1; dy <= 1; dy++) {
					Feature* f = eng->map->featuresStatic[pos.x + dx][pos.y + dy];

					if(f->getId() == feature_trap) {
						dynamic_cast<Trap*>(f)->playerTrySpotHidden();
					}
					if(f->getId() == feature_door) {
						dynamic_cast<Door*>(f)->playerTrySpotHidden();
					}
				}
			}

			//Any item in the inventory that can be identified?
			attemptIdentifyItems();
		}
	}

	//First aid?
	if(firstAidTurnsLeft == 0) {
		eng->log->addMessage("You finish applying first aid.");
		eng->renderer->flip();
		restoreHP(99999);
		firstAidTurnsLeft = -1;
	}
	if(firstAidTurnsLeft > 0) {
		firstAidTurnsLeft--;
		eng->gameTime->letNextAct();
	}

	//Waiting?
	if(waitTurnsLeft > 0) {
		waitTurnsLeft--;
		eng->gameTime->letNextAct();
	}

	//When this function ends, the system starts reading keys.

}

void Player::attemptIdentifyItems() {
	const vector<Item*>* const general = m_inventory->getGeneral();
	for(unsigned int i = 0; i < general->size(); i++) {
		Item* const item = general->at(i);
		const ItemDefinition& def = item->getInstanceDefinition();

		//It must not be a readable item (those must be actively identified)
		if(def.isReadable == false) {
			if(def.isIdentified == false) {
				if(def.abilityToIdentify != ability_empty) {
					const int SKILL = m_instanceDefinition.abilityValues.getAbilityValue(def.abilityToIdentify, true);
					if(SKILL > (100 - def.identifySkillFactor)) {
						item->setRealDefinitionNames(eng, false);
						eng->log->addMessage("You recognize " + def.name.name_a + " in your inventory.", clrWhite, true);
					}
				}
			}
		}
	}
}

void Player::queryInterruptActions() {
	//Abort searching
	if(waitTurnsLeft > 0) {
		eng->renderer->drawMapAndInterface();
		eng->log->addMessage("You stop waiting.", clrWhite, false);
		eng->renderer->flip();
	}
	waitTurnsLeft = -1;

	const bool IS_FAINTED = m_statusEffectsHandler->hasEffect(statusFainted);
	const bool IS_PARALYSED = m_statusEffectsHandler->hasEffect(statusParalyzed);
	const bool IS_DEAD = deadState != actorDeadState_alive;

	//If monster is in view, or player is paralysed, fainted or dead, abort first aid - else query abort
	if(firstAidTurnsLeft > 0) {
		getSpotedEnemies();
		if(spotedEnemies.size() > 0 || IS_FAINTED || IS_PARALYSED || IS_DEAD) {
			firstAidTurnsLeft = -1;
			eng->log->addMessage("You stop mending your wounds.", clrWhite, false);
			eng->renderer->flip();
		} else {
			eng->renderer->drawMapAndInterface();
			const string TURNS_STR = intToString(firstAidTurnsLeft);
			eng->log->addMessage("Continue applying first aid (" + TURNS_STR + " turns)? (y/n)", clrWhiteHigh, false);
			eng->renderer->flip();
			if(eng->query->yesOrNo() == false) {
				firstAidTurnsLeft = -1;
			}
			eng->log->clearLog();
			eng->renderer->flip();
		}
	}
}

void Player::explosiveThrown() {
	dynamiteFuseTurns = -1;
	molotovFuseTurns = -1;
	flareFuseTurns = -1;
	updateColor();
	eng->renderer->drawMapAndInterface();
}

void Player::registerHeardSound(const Sound& sound) {
	const coord origin = sound.getOrigin();
	const string message = sound.getMessage();
	if(message != "") {
		//Display the message if player does not see origin cell,
		//or if the message should be displayed despite this.
		const bool DISPLAY_MESSAGE =
         eng->map->playerVision[origin.x][origin.y] == false ||
         sound.getIsMessageIgnoredIfPlayerSeeCell() == false;

		if(DISPLAY_MESSAGE == true)
			eng->log->addMessage(message, clrYellow);
	}
}

void Player::moveDirection(const int X_DIR, const int Y_DIR) {
	if(deadState == actorDeadState_alive) {
		coord dest = m_statusEffectsHandler->changeMoveCoord(pos, pos + coord(X_DIR, Y_DIR));

		//See if trap affects leaving
		if(dest != pos) {
			Feature* f = eng->map->featuresStatic[pos.x][pos.y];
			if(f->getId() == feature_trap) {
				//TODO Consider moving actorAttemptLeave to base class
				Trap* trap = dynamic_cast<Trap*>(f);
				dest = trap->actorAttemptLeave(this, pos, dest);
			}
		}

		bool isSwiftMoveAlloed = false;

		if(dest != pos) {
		    // Attack?
			Actor* const actorAtDest = eng->mapTests->getActorAtPos(dest);
			if(actorAtDest != NULL) {
				if(m_statusEffectsHandler->allowAttackMelee(true) == true) {
					bool hasMeleeWeapon = false;
					Weapon* weapon = dynamic_cast<Weapon*>(m_inventory->getItemInSlot(slot_wielded));
					if(weapon != NULL) {
						if(weapon->getInstanceDefinition().isMeleeWeapon) {
							hasMeleeWeapon = true;
							eng->attack->melee(dest.x, dest.y, weapon);
							target = actorAtDest;
							return;
						}
					}
					if(hasMeleeWeapon == false) {
						eng->log->addMessage("[punch]");
					}
				}
				return;
			}

			// This point reached means no actor in the destination cell.

			// Blocking mobile or static features?
			bool featuresAllowMove = eng->map->featuresStatic[dest.x][dest.y]->isMovePassable(this);
			vector<FeatureMob*> featureMobs = eng->gameTime->getFeatureMobsAtPos(dest);
			if(featuresAllowMove) {
				for(unsigned int i = 0; i < featureMobs.size(); i++) {
					if(featureMobs.at(i)->isMovePassable(this) == false) {
						featuresAllowMove = false;
						break;
					}
				}
			}

			if(featuresAllowMove) {

			    // Encumbered?
			    if(m_inventory->getTotalItemWeight() >= PLAYER_CARRY_WEIGHT_STANDARD) {
                    eng->log->addMessage("You care too encumbered to move.");
                    eng->renderer->flip();
                    return;
			    }

				isSwiftMoveAlloed = true;
				const coord oldPos = pos;
				pos = dest;

				Item* const item = eng->map->items[pos.x][pos.y];
				if(item != NULL) {
					string message = m_statusEffectsHandler->allowSee() == false ? "You feel here: " : "You see here: ";
					message += eng->itemData->itemInterfaceName(item, true);
					eng->log->addMessage(message + ".");
				}

				if(m_statusEffectsHandler->allowSee() == false) {
					eng->map->playerVision[oldPos.x][oldPos.y] = true;
					FOVupdate();
					eng->map->playerVision[oldPos.x][oldPos.y] = false;
				}
			}

			// Note: bump() prints block messages.
			for(unsigned int i = 0; i < featureMobs.size(); i++) {
				featureMobs.at(i)->bump(this);
			}
			eng->map->featuresStatic[dest.x][dest.y]->bump(this);
		}
		//If moved successfully or for example was held by a spider web, end turn.
		if(pos == dest) {
			bool isFreeTurn = false;
			if(isSwiftMoveAlloed) {
				const int MOBILITY_SKILL = m_instanceDefinition.abilityValues.getAbilityValue(ability_mobility, true);
				if(eng->abilityRoll->roll(MOBILITY_SKILL) >= successSmall) {
					isFreeTurn = true;
//                    eng->log->addMessage("You move swiftly.", clrMagenta);
					eng->playerVisualMemory->updateVisualMemory();
					eng->player->FOVupdate();
					eng->renderer->drawMapAndInterface();
				}
			}
			if(isFreeTurn == false) {
				eng->gameTime->letNextAct();
			}
		} else {
			eng->renderer->flip();
		}
	}
}

void Player::autoMelee() {
	if(target != NULL) {
		if(eng->mapTests->isCellsNeighbours(pos, target->pos, false)) {
			if(checkIfSeeActor(*target, NULL)) {
				moveDirection(target->pos - pos);
				return;
			}
		}
	}

	//If this line reached, there is no adjacent current target.
	for(int dx = -1; dx <= 1; dx++) {
		for(int dy = -1; dy <= 1; dy++) {
			if(dx != 0 || dy != 0) {
				const Actor* const actor = eng->mapTests->getActorAtPos(pos + coord(dx, dy));
				if(actor != NULL) {
					if(checkIfSeeActor(*actor, NULL) == true) {
						target = actor;
						moveDirection(coord(dx, dy));
						return;
					}
				}
			}
		}
	}
}

void Player::kick() {
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeVisionBlockerArray(blockers);

	Actor* actorToKick = NULL;

	//If there is only one adjacent visible actor, kick it
	if(eng->config->ALWAYS_ASK_DIRECTION == false) {
		int adjacentActorCount = 0;
		for(unsigned int i = 0; i < eng->gameTime->getLoopSize(); i++) {
			Actor* actor = eng->gameTime->getActorAt(i);
			if(actor != this) {
				if(actor->deadState == actorDeadState_alive) {
					if(eng->mapTests->isCellsNeighbours(pos, actor->pos, false)) {
						if(checkIfSeeActor(*actor, blockers)) {
							adjacentActorCount++;
							if(adjacentActorCount == 1) {
								actorToKick = actor;
							} else {
								actorToKick = NULL;
								i = 9999999;
							}
						}
					}
				}
			}
		}
	}

	//If not exactly one adjacent visible actor was found, ask player for direction
	if(actorToKick == NULL) {
		eng->log->addMessage("Kick in what direction? [Space/Esc Cancel]", clrWhiteHigh);
		eng->renderer->flip();
		const coord delta = eng->query->direction();
		eng->log->clearLog();
		if(delta == coord(0, 0)) {
			return;
		} else {
			const coord kickPos = pos + delta;
			actorToKick = eng->mapTests->getActorAtPos(kickPos);

		}
	}

	if(actorToKick == NULL) {
		if(eng->player->getStatusEffectsHandler()->allowSee() == true) {
			eng->log->addMessage("You see no one there to kick.");
		} else {
			eng->log->addMessage("You find no one there to kick.");
		}
	} else {
		//Spawn a temporary kick weapon to attack with
		Weapon* kickWeapon = NULL;

		const ActorDefinition* const d = actorToKick->getInstanceDefinition();
		//If kicking critters, call it a stomp instead and give it bonus hit chance
		if(d->actorSize == actorSize_floor && (d->isSpider == true || d->isRat == true)) {
			kickWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerStomp));
		} else {
			kickWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerKick));
		}
		eng->attack->melee(actorToKick->pos.x, actorToKick->pos.y, kickWeapon);
	}
}

void Player::FOVupdate() {
	const unsigned int SIZE = eng->gameTime->getFeatureMobsSize();
	for(unsigned int i = 0; i < SIZE; i++) {
		eng->gameTime->getFeatureMobAt(i)->getLight(eng->map->light);
	}
	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			eng->map->featuresStatic[x][y]->getLight(eng->map->light);
		}
	}

	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			eng->map->playerVision[x][y] = false;
		}
	}

	if(m_statusEffectsHandler->allowSee()) {
		bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
		eng->mapTests->makeVisionBlockerArray(blockers);
		eng->fov->runPlayerFov(blockers, pos.x, pos.y);
		FOVhack();
	}

	eng->map->playerVision[pos.x][pos.y] = true;

	if(eng->cheat_vision) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			for(int x = 0; x < MAP_X_CELLS; x++) {
				eng->map->playerVision[x][y] = true;
			}
		}
	}

	//Explore
	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			if(eng->map->playerVision[x][y] == true) {
				eng->map->explored[x][y] = true;
			}
		}
	}
}

void Player::FOVhack() {
	bool visionBlocked[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeVisionBlockerArray(visionBlocked);

	bool moveBlocked[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeMoveBlockerArrayFeaturesOnly(eng->player, moveBlocked);

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			if(visionBlocked[x][y] && moveBlocked[x][y]) {
				for(int dy = -1; dy <= 1; dy++) {
					for(int dx = -1; dx <= 1; dx++) {
						if(eng->mapTests->isCellInsideMainScreen(coord(x + dx, y + dy)) == true) {
							if(eng->map->playerVision[x + dx][y + dy] == true && moveBlocked[x + dx][y + dy] == false) {
								eng->map->playerVision[x][y] = true;
								dx = 999;
								dy = 999;
							}
						}
					}
				}
			}
		}
	}
}
