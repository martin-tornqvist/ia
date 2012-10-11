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
#include "PlayerBonuses.h"
#include "FeatureLitDynamite.h"

Player::Player() :
  firstAidTurnsLeft(-1), waitTurnsLeft(-1), insanityLong(0), insanityShort(0), insanityShortTemp(0), arcaneKnowledge(0),
  dynamiteFuseTurns(-1), molotovFuseTurns(-1),
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

  const PlayerBackgrounds_t background = eng->playerCreateCharacter->getPlayerBackground();
  switch(background) {
  case playerBackground_soldier: {
    m_inventory->putItemInSlot(slot_armorBody, eng->itemFactory->spawnItem(item_armorFlackJacket), true, true);

    ItemDevNames_t weaponId;
    int wpnNr = eng->dice(1, 4);
    switch(wpnNr) {
    case 1: weaponId = item_hatchet; break;
    case 2: weaponId = item_machete; break;
    case 3: weaponId = item_club; break;
    case 4: weaponId = item_axe; break;
    }
    m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);

    wpnNr = eng->dice(1, 3);
    switch(wpnNr) {
    case 1: {
      m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_machineGun), true, true);
      m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_drumOfBullets));
    }
    break;
    case 2: {
      m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_sawedOff), true, true);
      Item* item = eng->itemFactory->spawnItem(item_shotgunShell);
      item->numberOfItems = eng->dice.getInRange(12, 16);
      m_inventory->putItemInGeneral(item);
    }
    break;
    case 3: {
      m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_pumpShotgun), true, true);
      Item* item = eng->itemFactory->spawnItem(item_shotgunShell);
      item->numberOfItems = eng->dice.getInRange(6, 10);
      m_inventory->putItemInGeneral(item);
    }
    break;
    }

    m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
    m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));

    Item* item = eng->itemFactory->spawnItem(item_dynamite);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_molotov);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_ironSpike);
    item->numberOfItems = eng->dice.getInRange(5, 8);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_flare);
    item->numberOfItems = eng->dice.getInRange(3, 5);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_throwingKnife);
    item->numberOfItems = eng->dice.getInRange(8, 12);
    m_inventory->putItemInSlot(slot_missiles, item, true, true);

    eng->playerBonusHandler->increaseBonusForAbility(ability_accuracyRanged);
    eng->playerBonusHandler->increaseBonusForAbility(ability_resistStatusBody);

    m_instanceDefinition.HP_max = 18;
    m_instanceDefinition.HP = m_instanceDefinition.HP_max;
  }
  break;

  case playerBackground_occultScholar: {
    for(int i = 0; i < 2; i++) {
      bool done = false;
      while(done == false) {
        Item* item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
        ItemDefinition& archDef = item->getArchetypeDefinition();
        if(archDef.isScrollLearned == false && archDef.isScrollLearnable) {
          item->setRealDefinitionNames(eng, true);
          done = archDef.isScrollLearned = true;
        }
        delete item;
        item = NULL;
      }
    }

    ItemDevNames_t weaponId;
    int wpnNr = eng->dice(1, 2);
    switch(wpnNr) {
    case 1: weaponId = item_dagger; break;
    case 2: weaponId = item_hatchet; break;
    }
    m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);

    m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
    m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));

    Item* item = eng->itemFactory->spawnItem(item_dynamite);
    item->numberOfItems = eng->dice.getInRange(1, 2);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_molotov);
    item->numberOfItems = eng->dice.getInRange(1, 2);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_ironSpike);
    item->numberOfItems = eng->dice.getInRange(5, 8);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_flare);
    item->numberOfItems = eng->dice.getInRange(3, 5);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    item->setRealDefinitionNames(eng, true);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    item->setRealDefinitionNames(eng, true);
    m_inventory->putItemInGeneral(item);

    eng->playerBonusHandler->increaseBonusForAbility(ability_language);
    eng->playerBonusHandler->increaseBonusForAbility(ability_resistStatusMind);

    m_instanceDefinition.HP_max = 14;
    m_instanceDefinition.HP = m_instanceDefinition.HP_max;
  }
  break;

  case playerBackground_tombRaider: {
    m_inventory->putItemInSlot(slot_armorBody, eng->itemFactory->spawnItem(item_armorLeatherJacket), true, true);

    Item* item = eng->itemFactory->spawnItem(item_dagger);
    dynamic_cast<Weapon*>(item)->setMeleePlus(2);
    m_inventory->putItemInSlot(slot_wielded, item, true, true);

    m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
    m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));

    item = eng->itemFactory->spawnItem(item_dynamite);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_molotov);
    item->numberOfItems = eng->dice.getInRange(2, 3);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_ironSpike);
    item->numberOfItems = eng->dice.getInRange(5, 8);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_flare);
    item->numberOfItems = eng->dice.getInRange(3, 5);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnItem(item_throwingKnife);
    item->numberOfItems = eng->dice.getInRange(8, 12);
    m_inventory->putItemInSlot(slot_missiles, item, true, true);

    item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
    item->numberOfItems = eng->dice.getInRange(1, 2);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(true, false);
    item->numberOfItems = eng->dice.getInRange(1, 2);
    item->setRealDefinitionNames(eng, true);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
    item->numberOfItems = eng->dice.getInRange(1, 2);
    m_inventory->putItemInGeneral(item);

    item = eng->itemFactory->spawnRandomScrollOrPotion(false, true);
    item->numberOfItems = eng->dice.getInRange(1, 2);
    item->setRealDefinitionNames(eng, true);
    m_inventory->putItemInGeneral(item);

    eng->playerBonusHandler->increaseBonusForAbility(ability_searching);
    eng->playerBonusHandler->increaseBonusForAbility(ability_stealth);

    m_instanceDefinition.HP_max = 16;
    m_instanceDefinition.HP = m_instanceDefinition.HP_max;
  }
  break;

  default: {} break;
  }



//	int NR_OF_CARTRIDGES = eng->dice.getInRange(1, 3);
//	int NR_OF_DYNAMITE = eng->dice.getInRange(2, 4);
//	int NR_OF_MOLOTOV = eng->dice.getInRange(2, 4);
//	int NR_OF_FLARES = eng->dice.getInRange(3, 5);
//	int NR_OF_THROWING_KNIVES = eng->dice.getInRange(7, 12);
//	int NR_OF_SPIKES = eng->dice.getInRange(3, 4);
//
//	const int NR_OF_POSSIBLE_WEAPONS = 6;
//
//	const int WEAPON_CHOICE = eng->dice.getInRange(1, NR_OF_POSSIBLE_WEAPONS);
//	ItemDevNames_t weaponId = item_dagger;
//	switch(WEAPON_CHOICE) {
//	case 1:
//		weaponId = item_dagger;
//		break;
//	case 2:
//		weaponId = item_hatchet;
//		break;
//	case 3:
//		weaponId = item_club;
//		break;
//	case 4:
//		weaponId = item_hammer;
//		break;
//	case 5:
//		weaponId = item_machete;
//		break;
//	case 6:
//		weaponId = item_axe;
//		break;
//
//	default:
//		weaponId = item_dagger;
//		break;
//	}
//
//	m_inventory->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);
//
//	m_inventory->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);
//
//	for(int i = 0; i < NR_OF_CARTRIDGES; i++) {
//		m_inventory->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
//	}
//
//	Item* item = eng->itemFactory->spawnItem(item_dynamite);
//	item->numberOfItems = NR_OF_DYNAMITE;
//	m_inventory->putItemInGeneral(item);
//
//	item = eng->itemFactory->spawnItem(item_molotov);
//	item->numberOfItems = NR_OF_MOLOTOV;
//	m_inventory->putItemInGeneral(item);
//
//	item = eng->itemFactory->spawnItem(item_flare);
//	item->numberOfItems = NR_OF_FLARES;
//	m_inventory->putItemInGeneral(item);
//
//	item = eng->itemFactory->spawnItem(item_throwingKnife);
//	item->numberOfItems = NR_OF_THROWING_KNIVES;
//	m_inventory->putItemInSlot(slot_missiles, item, true, true);
//
//	item = eng->itemFactory->spawnItem(item_ironSpike);
//	item->numberOfItems = NR_OF_SPIKES;
//	m_inventory->putItemInGeneral(item);
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
  lines.push_back(intToString(arcaneKnowledge));
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
    eng->log->addMessage("My applying of first aid is disrupted.", clrWhite, false);
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
  arcaneKnowledge = stringToInt(lines.front());
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
  const int PLAYER_FORTITUDE = m_instanceDefinition.abilityValues.getAbilityValue(ability_resistStatusMind, true);

  if(PLAYER_FORTITUDE < 99) {
    const int FORTITUDE_AT_RANK_ZERO = eng->playerBonusHandler->getBonusAbilityModifierAtRank(ability_resistStatusMind, 0);
    const int SHOCK_REDUCTION_PERCENT = min(100, max(0, PLAYER_FORTITUDE - FORTITUDE_AT_RANK_ZERO));

    int baseIncr = 0;
    switch(shockValue) {
    case shockValue_none: {
      baseIncr = 0;
    }
    break;
    case shockValue_mild: {
      baseIncr = eng->dice(1, 2);
    }
    break;
    case shockValue_some: {
      baseIncr = eng->dice(1, 4);
    }
    break;
    case shockValue_heavy: {
      baseIncr = eng->dice(2, 4) + 4;
    }
    break;
    }

    const int SHOCK_AFTER_FORTITUDE = (baseIncr * (100 - SHOCK_REDUCTION_PERCENT)) / 100;

    insanityShort = min(100, insanityShort + max(0, SHOCK_AFTER_FORTITUDE + MODIFIER));
  }
}

void Player::incrInsanityLong() {
  string popupMessage = "Insanity draws nearer... ";

  if(eng->config->BOT_PLAYING == false) {
    insanityLong += eng->dice(1, 4) + 2;
  }

  insanityShort = max(0, insanityShort - 60);

  updateColor();
  eng->renderer->drawMapAndInterface();

  if(insanityLong >= 100) {
    popupMessage += "My mind can no longer withstand what it has grasped. I am hopelessly lost.";
    eng->popup->showMessage(popupMessage, true);
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
        if(playerSeeShockingMonster) {
          if(eng->dice.coinToss()) {
            popupMessage += "I let out a terrified shriek.";
          } else {
            popupMessage += "I scream in terror.";
          }
          eng->popup->showMessage(popupMessage, true);
          eng->soundEmitter->emitSound(Sound("", true, pos, true, true));
          return;
        }
      }
      break;
      case 2: {
        popupMessage += "I find myself babbling incoherently.";
        eng->popup->showMessage(popupMessage, true);
        for(int i = eng->dice(1, 3) + 2; i > 0; i--) {
          const string* const phrase = eng->phrases->getAggroPhraseFromPhraseSet(phraseSet_cultist);
          eng->log->addMessage("[" + getNameThe() + "]" + *phrase);
        }
        eng->soundEmitter->emitSound(Sound("", true, pos, false, true));
        return;
      }
      break;
      case 3: {
        popupMessage += "I struggle to not fall into a stupor.";
        eng->popup->showMessage(popupMessage, true);
        m_statusEffectsHandler->attemptAddEffect(new StatusFainted(eng));
        return;
      }
      break;
      case 4: {
        popupMessage += "I laugh nervously.";
        eng->popup->showMessage(popupMessage, true);
        eng->soundEmitter->emitSound(Sound("", true, pos, false, true));
        return;
      }
      break;
      case 5: {
        popupMessage += "Thanks to the mercy of the mind, some past experiences are forgotten (-5% xp).";
        eng->popup->showMessage(popupMessage, true);
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
                popupMessage += "I am afflicted by Murophobia. Rats suddenly seem terrifying.";
                eng->popup->showMessage(popupMessage, true);
                insanityPhobias[insanityPhobia_rat] = true;
                return;
              }
              if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isSpider == true && insanityPhobias[insanityPhobia_spider]
                  == false) {
                popupMessage += "I am afflicted by Arachnophobia. Spiders suddenly seem terrifying.";
                eng->popup->showMessage(popupMessage, true);
                insanityPhobias[insanityPhobia_spider] = true;
                return;
              }
              if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isCanine == true && insanityPhobias[insanityPhobia_dog]
                  == false) {
                popupMessage += "I am afflicted by Cynophobia. Dogs suddenly seem terrifying.";
                eng->popup->showMessage(popupMessage, true);
                insanityPhobias[insanityPhobia_dog] = true;
                return;
              }
              if(spotedEnemies.at(MONSTER_ROLL)->getInstanceDefinition()->isUndead == true && insanityPhobias[insanityPhobia_undead]
                  == false) {
                popupMessage += "I am afflicted by Necrophobia. The undead suddenly seem much more terrifying.";
                eng->popup->showMessage(popupMessage, true);
                insanityPhobias[insanityPhobia_undead] = true;
                return;
              }
            }
          } else {
            if(eng->dice.coinToss()) {
              if(isStandingInOpenSpace()) {
                if(insanityPhobias[insanityPhobia_openPlace] == false) {
                  popupMessage += "I am afflicted by Agoraphobia. Open places suddenly seem terrifying.";
                  eng->popup->showMessage(popupMessage, true);
                  insanityPhobias[insanityPhobia_openPlace] = true;
                  return;
                }
              }
              if(isStandingInCrampedSpace()) {
                if(insanityPhobias[insanityPhobia_closedPlace] == false) {
                  popupMessage += "I am afflicted by Claustrophobia. Confined places suddenly seem terrifying.";
                  eng->popup->showMessage(popupMessage, true);
                  insanityPhobias[insanityPhobia_closedPlace] = true;
                  return;
                }
              }
            } else {
              if(eng->map->getDungeonLevel() >= 5) {
                if(insanityPhobias[insanityPhobia_deepPlaces] == false) {
                  popupMessage += "I am afflicted by Bathophobia. It suddenly seems terrifying to delve deeper.";
                  eng->popup->showMessage(popupMessage, true);
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
              += "To my alarm, I find myself encouraged by the sensation of pain. Every time I am hurt, I find a little relief. However, my depraved mind decays faster over time now.";
              eng->popup->showMessage(popupMessage, true);
              insanityCompulsions[insanityCompulsion_masochism] = true;
              return;
            }
            break;
            case insanityCompulsion_sadism: {
              popupMessage
              += "To my alarm, I find myself encouraged by the pain I cause in others. For every life I take, I find a little relief. However, my depraved mind decays faster over time now.";
              eng->popup->showMessage(popupMessage, true);
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
        popupMessage += "The shadows are closing in on me!";
        eng->popup->showMessage(popupMessage, true);

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

        int shadowsLeftToSpawn = eng->dice.getInRange(2, 3);
        while(shadowsLeftToSpawn > 0) {
          const int D_MAX = 3;
          for(int dx = -D_MAX; dx <= D_MAX; dx++) {
            for(int dy = -D_MAX; dy <= D_MAX; dy++) {
              if(dx <= -2 || dx >= 2 || dy <= -2 || dy >= 2) {
                if(blockers[pos.x + dx][pos.y + dy] == false) {
                  if(eng->dice(1, 100) < 10) {
                    Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_shadow, pos + coord(dx, dy)));
                    if(eng->dice.coinToss()) {
                      monster->isStealth = true;
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

void Player::setTempShockFromFeatures() {
  if(eng->map->darkness[pos.x][pos.y] && eng->map->light[pos.x][pos.y] == false) {
    insanityShortTemp += 20;
  }

  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      const Feature* const f = eng->map->featuresStatic[pos.x + dx][pos.y + dy];
      insanityShortTemp += f->getShockWhenAdjacent();
    }
  }
  insanityShortTemp = min(99, insanityShortTemp);
}

bool Player::isStandingInOpenSpace() const {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayFeaturesOnly(this, blockers);
  for(int y = pos.y - 1; y <= pos.y + 1; y++) {
    for(int x = pos.x - 1; x <= pos.x + 1; x++) {
      if(blockers[x][y]) {
        return false;
      }
    }
  }

  return true;
}

bool Player::isStandingInCrampedSpace() const {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayFeaturesOnly(this, blockers);
  int blockCount = 0;
  for(int y = pos.y - 1; y <= pos.y + 1; y++) {
    for(int x = pos.x - 1; x <= pos.x + 1; x++) {
      if(blockers[x][y]) {
        blockCount++;
        if(blockCount >= 6) {
          return true;
        }
      }
    }
  }

  return false;
}

void Player::testPhobias() {
  const int ROLL = eng->dice(1, 100);
  //Phobia vs creature type?
  if(ROLL < 10) {
    for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
      if(spotedEnemies.at(0)->getInstanceDefinition()->isCanine == true && insanityPhobias[insanityPhobia_dog] == true) {
        eng->log->addMessage("I am plagued by my canine phobia!");
        m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
      if(spotedEnemies.at(0)->getInstanceDefinition()->isRat == true && insanityPhobias[insanityPhobia_rat] == true) {
        eng->log->addMessage("I am plagued by my rat phobia!");
        m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
      if(spotedEnemies.at(0)->getInstanceDefinition()->isUndead == true && insanityPhobias[insanityPhobia_undead] == true) {
        eng->log->addMessage("I am plagued by my phobia of the dead!");
        m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
      if(spotedEnemies.at(0)->getInstanceDefinition()->isSpider == true && insanityPhobias[insanityPhobia_spider] == true) {
        eng->log->addMessage("I am plagued by my spider phobia!");
        m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
    }
  }
  if(ROLL < 5) {
    if(insanityPhobias[insanityPhobia_openPlace] == true) {
      if(isStandingInOpenSpace()) {
        eng->log->addMessage("I am plagued by my phobia of open places!");
        m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
    }

    if(insanityPhobias[insanityPhobia_closedPlace] == true) {
      if(isStandingInCrampedSpace()) {
        eng->log->addMessage("I am plagued by my phobia of closed places!");
        m_statusEffectsHandler->attemptAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
    }
  }

  if(eng->map->featuresStatic[pos.x][pos.y]->getId() == feature_stairsDown && insanityPhobias[insanityPhobia_deepPlaces]) {
    eng->log->addMessage("I am plagued by my phobia of deep places!");
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

  if(getHP() <= getHP_max() / 3 + 1) {
    clr = clrRed;
    return;
  }

  const int CUR_SHOCK = insanityShort + insanityShortTemp;
  if(CUR_SHOCK >= 75) {
    clr = clrMagenta;
    return;
  }

  clr = m_archetypeDefinition->color;
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
    eng->log->addMessage("The dynamite explodes in my hands!");
    eng->explosionMaker->runExplosion(pos);
    updateColor();
    dynamiteFuseTurns = -1;
  }

  //Molotovs
  if(molotovFuseTurns > 0) {
    molotovFuseTurns--;
  }
  if(molotovFuseTurns == 0) {
    eng->log->addMessage("The Molotov Cocktail explodes in my hands!");
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

  if(firstAidTurnsLeft == -1) {
    testPhobias();
  }

  //Lose short-term sanity from seen monsters?
  for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
    Monster* monster = dynamic_cast<Monster*>(spotedEnemies.at(i));
    const ActorDefinition* const def = monster->getInstanceDefinition();
    if(def->shockValue != shockValue_none) {
      shock(def->shockValue, -(monster->shockDecrease));
      monster->shockDecrease++;
    }
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
        eng->popup->showMessage("I have a bad feeling...", true);
      } else {
        eng->popup->showMessage("A chill runs down my spine...", true);
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
    incrInsanityLong();
    eng->gameTime->letNextAct();
    return;
  }

  const unsigned int LOOP_SIZE = eng->gameTime->getLoopSize();

  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    //If applying first aid etc, messages may be printed for monsters that comes into view.
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
              eng->log->addMessage(MONSTER_NAME + " comes into my view.");
            }
            monster->messageMonsterInViewPrinted = true;
          }
        } else {
          monster->messageMonsterInViewPrinted = false;

          //Is the monster sneaking? Try to spot it
          if(eng->map->playerVision[monster->pos.x][monster->pos.y]) {
            if(monster->isStealth) {
              const int PLAYER_SEARCH_SKILL = m_instanceDefinition.abilityValues.getAbilityValue(ability_searching, true);
              const AbilityRollResult_t rollResult = eng->abilityRoll->roll(PLAYER_SEARCH_SKILL);
              if(rollResult == successSmall) {
                eng->log->addMessage("I see something moving in the shadows.");
              }
              else if(rollResult > successSmall) {
                monster->isStealth = false;
                FOVupdate();
                eng->renderer->drawMapAndInterface();
                eng->log->addMessage("I spot " + monster->getNameA() + "!");
              }
            }
          }
        }
      }
    }
  }

  if(firstAidTurnsLeft == -1) {

    //Passive HP-regeneration from high first aid?
    if(eng->playerBonusHandler->getBonusRankForAbility(ability_firstAid) >= 3) {
      if(m_statusEffectsHandler->hasEffect(statusDiseased) == false) {
        const int REGEN_N_TURN = 8;
        if((TURN / REGEN_N_TURN) * REGEN_N_TURN == TURN && TURN > 1) {
          if(getHP() < getHP_max()) {
            m_instanceDefinition.HP++;
          }
        }
      }
    }

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
  // Old way:
  if(firstAidTurnsLeft == 0) {
    eng->log->clearLog();
    eng->log->addMessage("I finish applying first aid.");
    eng->renderer->flip();
    restoreHP(99999);
    if(eng->playerBonusHandler->getBonusRankForAbility(ability_firstAid) >= 2) {
      m_statusEffectsHandler->endEffect(statusDiseased);
    }
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
  //	const vector<Item*>* const general = m_inventory->getGeneral();
  //	for(unsigned int i = 0; i < general->size(); i++) {
  //		Item* const item = general->at(i);
  //		const ItemDefinition& def = item->getInstanceDefinition();
  //
  //		//It must not be a readable item (those must be actively identified)
  //		if(def.isReadable == false) {
  //			if(def.isIdentified == false) {
  //				if(def.abilityToIdentify != ability_empty) {
  //					const int SKILL = m_instanceDefinition.abilityValues.getAbilityValue(def.abilityToIdentify, true);
  //					if(SKILL > (100 - def.identifySkillFactor)) {
  //						item->setRealDefinitionNames(eng, false);
  //						eng->log->addMessage("I recognize " + def.name.name_a + " in my inventory.", clrWhite, true);
  //					}
  //				}
  //			}
  //		}
  //	}
}

int Player::getHealingTimeTotal() const {
  const int TURNS_BEFORE_BON = 70;
  const int PLAYER_HEALING_RANK = eng->playerBonusHandler->getBonusRankForAbility(ability_firstAid);
  return PLAYER_HEALING_RANK >= 1 ? TURNS_BEFORE_BON / 2 : TURNS_BEFORE_BON;
}

void Player::queryInterruptActions() {
  //Abort searching
  if(waitTurnsLeft > 0) {
    eng->renderer->drawMapAndInterface();
    eng->log->addMessage("I stop waiting.", clrWhite, false);
    eng->renderer->flip();
  }
  waitTurnsLeft = -1;

  const bool IS_FAINTED = m_statusEffectsHandler->hasEffect(statusFainted);
  const bool IS_PARALYSED = m_statusEffectsHandler->hasEffect(statusParalyzed);
  const bool IS_DEAD = deadState != actorDeadState_alive;

  //If monster is in view, or player is paralysed, fainted or dead, abort first aid - else query abort
  if(firstAidTurnsLeft > 0) {
    getSpotedEnemies();
    const int TOTAL_TURNS = getHealingTimeTotal();
    const bool IS_ENOUGH_TIME_PASSED = firstAidTurnsLeft < TOTAL_TURNS - 10;
    const int MISSING_HP = getHP_max() - getHP();
    const int HP_HEALED_IF_ABORTED = IS_ENOUGH_TIME_PASSED ? (MISSING_HP * (TOTAL_TURNS - firstAidTurnsLeft)) / TOTAL_TURNS  : 0;

    bool isAborted = false;
    if(spotedEnemies.size() > 0 || IS_FAINTED || IS_PARALYSED || IS_DEAD) {
      firstAidTurnsLeft = -1;
      isAborted = true;
      eng->log->addMessage("I stop tending to my wounds.", clrWhite, false);
      eng->renderer->flip();
    } else {
      eng->renderer->drawMapAndInterface();
      const string TURNS_STR = intToString(firstAidTurnsLeft);
      const string ABORTED_HP_STR = intToString(HP_HEALED_IF_ABORTED);
      string abortStr = "Continue healing (" + TURNS_STR + " turns)? (y/n), ";
      abortStr += ABORTED_HP_STR + " HP restored if canceled.";
      eng->log->addMessage(abortStr , clrWhiteHigh, false);
      eng->renderer->flip();
      if(eng->query->yesOrNo() == false) {
        firstAidTurnsLeft = -1;
        isAborted = true;
      }
      eng->log->clearLog();
      eng->renderer->flip();
    }
    if(isAborted && IS_ENOUGH_TIME_PASSED) {
      restoreHP(HP_HEALED_IF_ABORTED);
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
    //Display the message if player does not see origin cell, or if the message should be displayed despite this.
    if(eng->map->playerVision[origin.x][origin.y] == false || sound.getIsMessageIgnoredIfPlayerSeeOrigin() == false) {
      eng->log->addMessage(message);
    }
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
              if(eng->config->RANGED_WPN_MELEE_PROMPT) {
                if(weapon->getInstanceDefinition().isRangedWeapon) {
                  const string wpnName = weapon->getInstanceDefinition().name.name_a;
                  eng->log->addMessage("Attack " + actorAtDest->getNameThe() + " with " + wpnName + "? (y/n)", clrWhiteHigh);
                  eng->renderer->flip();
                  if(eng->query->yesOrNo() == false) {
                    eng->log->clearLog();
                    eng->renderer->flip();
                    return;
                  }
                }
              }
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
          eng->log->addMessage("I am too encumbered to move!");
          eng->renderer->flip();
          return;
        }

        isSwiftMoveAlloed = true;
        const coord oldPos = pos;
        pos = dest;

        Item* const item = eng->map->items[pos.x][pos.y];
        if(item != NULL) {
          string message = m_statusEffectsHandler->allowSee() == false ? "I feel here: " : "I see here: ";
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

  if(actorToKick == NULL) {
    if(eng->player->getStatusEffectsHandler()->allowSee() == true) {
      eng->log->addMessage("I see no one there to kick.");
    } else {
      eng->log->addMessage("I find no one there to kick.");
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

void Player::actorSpecific_addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {
  if(flareFuseTurns > 0) {
    bool myLight[MAP_X_CELLS][MAP_Y_CELLS];
    eng->basicUtils->resetBoolArray(myLight, false);
    const int RADI = LitFlare::getLightRadius();
    int x0 = pos.x - RADI;
    int y0 = pos.y - RADI;
    int x1 = pos.x + RADI;
    int y1 = pos.y + RADI;
    x0 = max(0, x0);
    y0 = max(0, y0);
    x1 = min(MAP_X_CELLS - 1, x1);
    y1 = min(MAP_Y_CELLS - 1, y1);
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    for(int y = y0; y <= y1; y++) {
      for(int x = x0; x <= x1; x++) {
        visionBlockers[x][y] = !eng->map->featuresStatic[x][y]->isVisionPassable();
      }
    }

    eng->fov->runFovOnArray(visionBlockers, pos, myLight, false);
    for(int y = y0; y <= y1; y++) {
      for(int x = x0; x <= x1; x++) {
        if(myLight[x][y]) {
          light[x][y] = true;
        }
      }
    }
  }
}

void Player::FOVupdate() {
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();

  addLight(eng->map->light);

  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    eng->gameTime->getFeatureMobAt(i)->addLight(eng->map->light);
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->map->featuresStatic[x][y]->addLight(eng->map->light);
      eng->map->playerVision[x][y] = false;
    }
  }

  if(m_statusEffectsHandler->allowSee()) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeVisionBlockerArray(blockers);
    eng->fov->runPlayerFov(blockers, pos);
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
            if(eng->mapTests->isCellInsideMainScreen(coord(x + dx, y + dy))) {
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
