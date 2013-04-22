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
#include "ItemDevice.h"

Player::Player() :
  firstAidTurnsLeft(-1), waitTurnsLeft(-1), dynamiteFuseTurns(-1),
  molotovFuseTurns(-1), flareFuseTurns(-1),
  target(NULL), insanity_(0), shock_(0.0), shockTemp_(0.0),
  mth(0), nrMovesUntilFreeAction(-1), carryWeightBase(450) {
}

void Player::actorSpecific_spawnStartItems() {
  def_->abilityVals.reset();

  for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
    insanityPhobias[i] = false;
  }
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    insanityObsessions[i] = false;
  }

  int NR_CARTRIDGES        = eng->dice.getInRange(1, 2);
  int NR_DYNAMITE          = eng->dice.getInRange(2, 3);
  int NR_MOLOTOV           = eng->dice.getInRange(2, 3);
//  int NR_FLARES            = eng->dice.getInRange(1, 6);
  int NR_THROWING_KNIVES   = eng->dice.getInRange(7, 12);
  int NR_SPIKES            = 0; //eng->dice.coinToss() ? 0 : eng->dice.getInRange(3, 4);

  const int WEAPON_CHOICE = eng->dice.getInRange(1, 5);
  ItemId_t weaponId = item_dagger;
  switch(WEAPON_CHOICE) {
    case 1:   weaponId = item_dagger;   break;
    case 2:   weaponId = item_hatchet;  break;
    case 3:   weaponId = item_hammer;   break;
    case 4:   weaponId = item_machete;  break;
    case 5:   weaponId = item_axe;      break;
    default:  weaponId = item_dagger;   break;
  }

//  inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);
  inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_sledgeHammer), true, true);
  inventory_->putItemInSlot(slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);

  for(int i = 0; i < NR_CARTRIDGES; i++) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
  }

  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_dynamite, NR_DYNAMITE));
  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_molotov, NR_MOLOTOV));

//  if(NR_FLARES > 0) {
//    item = eng->itemFactory->spawnItem(item_flare);
//    item->numberOfItems = NR_FLARES;
//    inventory_->putItemInGeneral(item);
//  }

  if(NR_THROWING_KNIVES > 0) {
    inventory_->putItemInSlot(slot_missiles, eng->itemFactory->spawnItem(item_throwingKnife, NR_THROWING_KNIVES), true, true);
  }

  if(NR_SPIKES > 0) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_ironSpike, NR_SPIKES));
  }

  inventory_->putItemInSlot(slot_armorBody, eng->itemFactory->spawnItem(item_armorLeatherJacket), true, true);

  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceElectricLantern));

//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceSentry));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceRejuvenator));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceRepeller));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceTranslocator));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_scrollOfIdentify, 4));
}

void Player::addSaveLines(vector<string>& lines) const {
  const unsigned int NR_STATUS_EFFECTS = statusEffectsHandler_->effects.size();
  lines.push_back(intToString(NR_STATUS_EFFECTS));
  for(unsigned int i = 0; i < NR_STATUS_EFFECTS; i++) {
    lines.push_back(intToString(statusEffectsHandler_->effects.at(i)->getEffectId()));
    lines.push_back(intToString(statusEffectsHandler_->effects.at(i)->turnsLeft));
  }

  lines.push_back(intToString(insanity_));
  lines.push_back(intToString(static_cast<int>(shock_)));
  lines.push_back(intToString(mth));
  lines.push_back(intToString(hp_));
  lines.push_back(intToString(hpMax_));
  lines.push_back(intToString(pos.x));
  lines.push_back(intToString(pos.y));
  lines.push_back(intToString(dynamiteFuseTurns));
  lines.push_back(intToString(molotovFuseTurns));
  lines.push_back(intToString(flareFuseTurns));

  for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
    lines.push_back(insanityPhobias[i] == 0 ? "0" : "1");
  }
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    lines.push_back(insanityObsessions[i] == 0 ? "0" : "1");
  }
}

void Player::setParametersFromSaveLines(vector<string>& lines) {
  const unsigned int NR_STATUS_EFFECTS = stringToInt(lines.front());
  lines.erase(lines.begin());
  for(unsigned int i = 0; i < NR_STATUS_EFFECTS; i++) {
    const StatusEffects_t id = static_cast<StatusEffects_t>(stringToInt(lines.front()));
    lines.erase(lines.begin());
    const int TURNS = stringToInt(lines.front());
    lines.erase(lines.begin());
    statusEffectsHandler_->tryAddEffect(statusEffectsHandler_->makeEffectFromId(id, TURNS), true, true);
  }

  insanity_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  shock_ = static_cast<double>(stringToInt(lines.front()));
  lines.erase(lines.begin());
  mth = stringToInt(lines.front());
  lines.erase(lines.begin());
  hp_ = stringToInt(lines.front());
  lines.erase(lines.begin());
  hpMax_ = stringToInt(lines.front());
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
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    insanityObsessions[i] = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());
  }
}

void Player::actorSpecific_hit(const int DMG) {
  //Hit aborts first aid
  if(firstAidTurnsLeft != -1) {
    firstAidTurnsLeft = -1;
    eng->log->addMessage("My applying of first aid is disrupted.", clrWhite, messageInterrupt_force);
  }

  if(insanityObsessions[insanityObsession_masochism]) {
    if(DMG > 1) {
      shock_ = max(0.0, shock_ - 5.0);
    }
  } else {
    incrShock(1);
  }

  eng->renderer->drawMapAndInterface();
}

int Player::getCarryWeightLimit() const {
  const bool IS_TOUGH = eng->playerBonusHandler->isBonusPicked(playerBonus_tough);
  const bool IS_RUGGED = eng->playerBonusHandler->isBonusPicked(playerBonus_rugged);
  const bool IS_STRONG_BACKED = eng->playerBonusHandler->isBonusPicked(playerBonus_strongBacked);
  const bool IS_WEAK = statusEffectsHandler_->hasEffect(statusWeak);
  const int CARRY_WEIGHT_MOD = IS_TOUGH * 10 + IS_RUGGED * 10 + IS_STRONG_BACKED * 30 - IS_WEAK * 15;

  return (carryWeightBase * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::getShockResistance() const {
  int ret = 0;
  if(eng->playerBonusHandler->isBonusPicked(playerBonus_strongMinded)) {
    ret += 5;
  }
  if(eng->playerBonusHandler->isBonusPicked(playerBonus_unyielding)) {
    ret += 5;
  }
  if(eng->playerBonusHandler->isBonusPicked(playerBonus_coolHeaded)) {
    ret += 20;
  }
  return min(100, max(0, ret));
}

void Player::incrShock(const int VAL) {
  const double SHOCK_RES_DB = static_cast<double>(getShockResistance());
  const double VAL_DB = static_cast<double>(VAL);
  const double VAL_AFTER_SHOCK_RES = (VAL_DB * (100.0 - SHOCK_RES_DB)) / 100.0;
  shock_ = min(100.0, shock_ + max(0.0, VAL_AFTER_SHOCK_RES));
}

void Player::incrShock(const ShockValues_t shockValue) {
  const int PLAYER_FORTITUDE = def_->abilityVals.getVal(ability_resistStatusMind, true, *this);

  if(PLAYER_FORTITUDE < 99) {
    switch(shockValue) {
      case shockValue_none: {
        incrShock(0);
      }
      break;
      case shockValue_mild: {
        incrShock(2);
      }
      break;
      case shockValue_some: {
        incrShock(4);
      }
      break;
      case shockValue_heavy: {
        incrShock(8);
      }
      break;
      default:
      {} break;
    }
  }
}

void Player::restoreShock(const int amountRestored, const bool IS_TEMP_SHOCK_RESTORED) {
  // If an obsession is active, only restore to a certain min level
  bool isObsessionActive = 0;
  for(int i = 0; i < endOfInsanityObsessions; i++) {
    if(insanityObsessions[i]) {
      isObsessionActive = true;
      break;
    }
  }
  shock_ = max((isObsessionActive ? static_cast<double>(MIN_SHOCK_WHEN_OBSESSION) : 0.0), shock_ - amountRestored);
  shockTemp_ = IS_TEMP_SHOCK_RESTORED ? 0 : shockTemp_;
}

void Player::incrMth(const int VAL) {
  mth = max(0, min(100, mth + VAL));
  insanity_ = max(0, min(100, insanity_ + VAL / 2));
}

void Player::incrInsanity() {
  string popupMessage = getInsanity() < 100 ? "Insanity draws nearer... " : "";

  const int INS_INCR = 6;

  if(eng->config->BOT_PLAYING == false) {
    insanity_ += INS_INCR;
  }

  mth = min(100, mth + INS_INCR / 2);

  restoreShock(70, false);

  updateColor();
  eng->renderer->drawMapAndInterface();

  if(getInsanity() >= 100) {
    popupMessage += "My mind can no longer withstand what it has grasped. I am hopelessly lost.";
    eng->popup->showMessage(popupMessage, true, "Complete insanity!");
    die(true, false, false);
  } else {
    bool playerSeeShockingMonster = false;
    getSpotedEnemies();
    for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
      const ActorDefinition* const def = spotedEnemies.at(i)->getDef();
      if(def->monsterShockLevel != monsterShockLevel_none) {
        playerSeeShockingMonster = true;
      }
    }

    //When long term sanity decreases something happens (mostly bad)
    //(Reroll until something actually happens)
    for(unsigned int insAttemptCount = 0; insAttemptCount < 10000; insAttemptCount++) {
      const int ROLL = eng->dice(1, 9);
      switch(ROLL) {
        case 1: {
          if(playerSeeShockingMonster) {
            if(eng->dice.coinToss()) {
              popupMessage += "I let out a terrified shriek.";
            } else {
              popupMessage += "I scream in terror.";
            }
            eng->popup->showMessage(popupMessage, true, "Screaming!");
            eng->soundEmitter->emitSound(Sound("", true, pos, true, true));
            return;
          }
        }
        break;
        case 2: {
          popupMessage += "I find myself babbling incoherently.";
          eng->popup->showMessage(popupMessage, true, "Babbling!");
          for(int i = eng->dice.getInRange(3, 5); i > 0; i--) {
            const string phrase = Cultist::getCultistPhrase(eng);
            eng->log->addMessage(getNameThe() + ": " + phrase);
          }
          eng->soundEmitter->emitSound(Sound("", true, pos, false, true));
          return;
        }
        break;
        case 3: {
          popupMessage += "I struggle to not fall into a stupor.";
          eng->popup->showMessage(popupMessage, true, "Fainting!");
          statusEffectsHandler_->tryAddEffect(new StatusFainted(eng));
          return;
        }
        break;
        case 4: {
          popupMessage += "I laugh nervously.";
          eng->popup->showMessage(popupMessage, true, "HAHAHA!");
          eng->soundEmitter->emitSound(Sound("", true, pos, false, true));
          return;
        }
        break;
        case 5: {
          popupMessage += "Thanks to the mercy of the mind, some past experiences are forgotten (lost 10% of current XP).";
          eng->popup->showMessage(popupMessage, true, "Suppressing memories!");
          eng->dungeonMaster->playerLoseXpPercent(10);
          return;
        }
        break;
        case 6: {
          if(insanity_ > 5) {
            //There is a limit to the number of phobias you can have
            int phobiasActive = 0;
            for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
              if(insanityPhobias[i] == true) {
                phobiasActive++;
              }
            }
            if(phobiasActive < 2) {
              if(eng->dice.coinToss()) {
                if(spotedEnemies.size() > 0) {
                  const int MONSTER_ROLL = eng->dice(1, spotedEnemies.size()) - 1;
                  if(spotedEnemies.at(MONSTER_ROLL)->getDef()->isRat == true && insanityPhobias[insanityPhobia_rat] == false) {
                    popupMessage += "I am afflicted by Murophobia. Rats suddenly seem terrifying.";
                    eng->popup->showMessage(popupMessage, true, "Murophobia!");
                    insanityPhobias[insanityPhobia_rat] = true;
                    return;
                  }
                  if(spotedEnemies.at(MONSTER_ROLL)->getDef()->isSpider == true && insanityPhobias[insanityPhobia_spider] == false) {
                    popupMessage += "I am afflicted by Arachnophobia. Spiders suddenly seem terrifying.";
                    eng->popup->showMessage(popupMessage, true, "Arachnophobia!");
                    insanityPhobias[insanityPhobia_spider] = true;
                    return;
                  }
                  if(spotedEnemies.at(MONSTER_ROLL)->getDef()->isCanine == true && insanityPhobias[insanityPhobia_dog] == false) {
                    popupMessage += "I am afflicted by Cynophobia. Dogs suddenly seem terrifying.";
                    eng->popup->showMessage(popupMessage, true, "Cynophobia!");
                    insanityPhobias[insanityPhobia_dog] = true;
                    return;
                  }
                  if(spotedEnemies.at(MONSTER_ROLL)->getDef()->isUndead == true && insanityPhobias[insanityPhobia_undead] == false) {
                    popupMessage += "I am afflicted by Necrophobia. The undead suddenly seem much more terrifying.";
                    eng->popup->showMessage(popupMessage, true, "Necrophobia!");
                    insanityPhobias[insanityPhobia_undead] = true;
                    return;
                  }
                }
              } else {
                if(eng->dice.coinToss()) {
                  if(isStandingInOpenSpace()) {
                    if(insanityPhobias[insanityPhobia_openPlace] == false) {
                      popupMessage += "I am afflicted by Agoraphobia. Open places suddenly seem terrifying.";
                      eng->popup->showMessage(popupMessage, true, "Agoraphobia!");
                      insanityPhobias[insanityPhobia_openPlace] = true;
                      return;
                    }
                  }
                  if(isStandingInCrampedSpace()) {
                    if(insanityPhobias[insanityPhobia_closedPlace] == false) {
                      popupMessage += "I am afflicted by Claustrophobia. Confined places suddenly seem terrifying.";
                      eng->popup->showMessage(popupMessage, true, "Claustrophobia!");
                      insanityPhobias[insanityPhobia_closedPlace] = true;
                      return;
                    }
                  }
                } else {
                  if(eng->map->getDungeonLevel() >= 5) {
                    if(insanityPhobias[insanityPhobia_deepPlaces] == false) {
                      popupMessage += "I am afflicted by Bathophobia. It suddenly seems terrifying to delve deeper.";
                      eng->popup->showMessage(popupMessage, true, "Bathophobia!");
                      insanityPhobias[insanityPhobia_deepPlaces] = true;
                      return;
                    }
                  }
                }
              }
            }
          }
        }
        break;

        case 7: {
          if(insanity_ > 20) {
            int obsessionsActive = 0;
            for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
              if(insanityObsessions[i] == true) {
                obsessionsActive++;
              }
            }
            if(obsessionsActive == 0) {
              const InsanityObsession_t obsession = static_cast<InsanityObsession_t>(eng->dice.getInRange(0, endOfInsanityObsessions - 1));
              switch(obsession) {
                case insanityObsession_masochism: {
                  popupMessage += "To my alarm, I find myself encouraged by the sensation of pain. Every time I am hurt, ";
                  popupMessage += "I find a little relief. However, my depraved mind can no longer find complete peace ";
                  popupMessage += "(shock cannot go below " + intToString(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  eng->popup->showMessage(popupMessage, true, "Masochistic obsession!");
                  insanityObsessions[insanityObsession_masochism] = true;
                  return;
                }
                break;
                case insanityObsession_sadism: {
                  popupMessage += "To my alarm, I find myself encouraged by the pain I cause in others. For every life I take, ";
                  popupMessage += "I find a little relief. However, my depraved mind can no longer find complete peace ";
                  popupMessage += "(shock cannot go below " + intToString(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  eng->popup->showMessage(popupMessage, true, "Sadistic obsession!");
                  insanityObsessions[insanityObsession_sadism] = true;
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
          if(insanity_ > 8) {
            popupMessage += "The shadows are closing in on me!";
            eng->popup->showMessage(popupMessage, true, "Haunted by shadows!");

            bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
            eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

            vector<coord> spawnPosCandidates;

            const int D_MAX = 3;
            for(int dx = -D_MAX; dx <= D_MAX; dx++) {
              for(int dy = -D_MAX; dy <= D_MAX; dy++) {
                if(dx <= -2 || dx >= 2 || dy <= -2 || dy >= 2) {
                  if(blockers[pos.x + dx][pos.y + dy] == false) {
                    spawnPosCandidates.push_back(pos + coord(dx, dy));
                  }
                }
              }
            }
            const int NR_SPAWN_POS_CANDIDATES = spawnPosCandidates.size();
            if(NR_SPAWN_POS_CANDIDATES > 0) {
              const int NR_SHADOWS_TO_SPAWN = min(NR_SPAWN_POS_CANDIDATES, eng->dice(1, min(5, (eng->map->getDungeonLevel() + 1) / 2)));
              for(int i = 0; i < NR_SHADOWS_TO_SPAWN; i++) {
                const unsigned int SPAWN_POS_ELEMENT = eng->dice.getInRange(0, spawnPosCandidates.size() - 1);
                const coord spawnPos = spawnPosCandidates.at(SPAWN_POS_ELEMENT);
                spawnPosCandidates.erase(spawnPosCandidates.begin() + SPAWN_POS_ELEMENT);
                Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_shadow, spawnPos));
                monster->playerAwarenessCounter = monster->getDef()->nrTurnsAwarePlayer;
                if(eng->dice.coinToss()) {
                  monster->isStealth = true;
                }
              }
            }
            return;
          }
        }
        break;

        case 9: {
          if(eng->playerBonusHandler->isBonusPicked(playerBonus_selfAware) == false) {
            popupMessage += "I find myself in a peculiar detached daze, a tranced state of mind. I am not sure where I am, or what I am doing exactly.";
            eng->popup->showMessage(popupMessage, true, "Confusion!");
            statusEffectsHandler_->tryAddEffect(new StatusConfused(eng), true);
            return;
          }
        }
        break;

        default: {
        }
        break;
      }
    }
  }
}

void Player::setTempShockFromFeatures() {
  if(eng->map->darkness[pos.x][pos.y] && eng->map->light[pos.x][pos.y] == false) {
    shockTemp_ += 20;
  }

  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      const Feature* const f = eng->map->featuresStatic[pos.x + dx][pos.y + dy];
      shockTemp_ += f->getShockWhenAdjacent();
    }
  }
  shockTemp_ = min(99.0, shockTemp_);
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
  const int ROLL = eng->dice.percentile();
  //Phobia vs creature type?
  if(ROLL < 10) {
    for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
      if(spotedEnemies.at(0)->getDef()->isCanine == true && insanityPhobias[insanityPhobia_dog] == true) {
        eng->log->addMessage("I am plagued by my canine phobia!");
        statusEffectsHandler_->tryAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
      if(spotedEnemies.at(0)->getDef()->isRat == true && insanityPhobias[insanityPhobia_rat] == true) {
        eng->log->addMessage("I am plagued by my rat phobia!");
        statusEffectsHandler_->tryAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
      if(spotedEnemies.at(0)->getDef()->isUndead == true && insanityPhobias[insanityPhobia_undead] == true) {
        eng->log->addMessage("I am plagued by my phobia of the dead!");
        statusEffectsHandler_->tryAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
      if(spotedEnemies.at(0)->getDef()->isSpider == true && insanityPhobias[insanityPhobia_spider] == true) {
        eng->log->addMessage("I am plagued by my spider phobia!");
        statusEffectsHandler_->tryAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
    }
  }
  if(ROLL < 5) {
    if(insanityPhobias[insanityPhobia_openPlace] == true) {
      if(isStandingInOpenSpace()) {
        eng->log->addMessage("I am plagued by my phobia of open places!");
        statusEffectsHandler_->tryAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
    }

    if(insanityPhobias[insanityPhobia_closedPlace] == true) {
      if(isStandingInCrampedSpace()) {
        eng->log->addMessage("I am plagued by my phobia of closed places!");
        statusEffectsHandler_->tryAddEffect(new StatusTerrified(eng->dice(1, 6)));
        return;
      }
    }
  }
}

int Player::getHpMax(const bool WITH_MODIFIERS) const {
  if(WITH_MODIFIERS) {
    if(statusEffectsHandler_->hasEffect(statusDiseased)) {
      return (hpMax_ * 3) / 4;
    }
  }
  return hpMax_;
}

void Player::updateColor() {
  if(deadState != actorDeadState_alive) {
    clr_ = clrRed;
    return;
  }

  const sf::Color clrFromStatusEffect = statusEffectsHandler_->getColor();
  if(clrFromStatusEffect.r != 0 || clrFromStatusEffect.g != 0 || clrFromStatusEffect.b != 0) {
    clr_ = clrFromStatusEffect;
    return;
  }

  if(dynamiteFuseTurns > 0 || molotovFuseTurns > 0 || flareFuseTurns > 0) {
    clr_ = clrYellow;
    return;
  }

//  if(getHp() <= getHpMax() / 3 + 1) {
//    clr_ = clrRed;
//    return;
//  }

  const int CUR_SHOCK = shock_ + shockTemp_;
  if(CUR_SHOCK >= 75) {
    clr_ = clrMagenta;
    return;
  }

  clr_ = def_->color;
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
    molotovFuseTurns = -1;
    updateColor();
    eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng));
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

  //If obsessions are active, raise shock to a minimum level
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    if(insanityObsessions[i] == true) {
      shock_ = max(static_cast<double>(MIN_SHOCK_WHEN_OBSESSION), shock_);
      break;
    }
  }

  //Shock from seen monsters
  double shockFromMonstersCurrentPlayerTurn = 0.0;
  for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
    Monster* monster = dynamic_cast<Monster*>(spotedEnemies.at(i));
    const ActorDefinition* const def = monster->getDef();
    if(def->monsterShockLevel != monsterShockLevel_none) {
      switch(def->monsterShockLevel) {
        case monsterShockLevel_unsettling: {
          monster->shockCausedCurrent += 0.10;
          monster->shockCausedCurrent = min(monster->shockCausedCurrent + 0.05, 1.0);
        }
        break;
        case monsterShockLevel_scary: {
          monster->shockCausedCurrent = min(monster->shockCausedCurrent + 0.15, 1.0);
        }
        break;
        case monsterShockLevel_terrifying: {
          monster->shockCausedCurrent = min(monster->shockCausedCurrent + 0.5, 2.0);
        }
        break;
        case monsterShockLevel_mindShattering: {
          monster->shockCausedCurrent = min(monster->shockCausedCurrent + 0.75, 3.0);
        }
        break;
        default:
        {} break;
      }
      if(shockFromMonstersCurrentPlayerTurn < 3.0) {
        incrShock(static_cast<int>(floor(monster->shockCausedCurrent)));
        shockFromMonstersCurrentPlayerTurn += monster->shockCausedCurrent;
      }
    }
  }

  //Some shock is taken every Xth turn
  const int TURN = eng->gameTime->getTurn();
  const int LOSE_N_TURN = 14;
  if((TURN / LOSE_N_TURN) * LOSE_N_TURN == TURN && TURN > 1) {
    if(eng->dice(1, 1000) <= 2) {
      if(eng->dice.coinToss()) {
        eng->popup->showMessage("I have a bad feeling about this...", true);
      } else {
        eng->popup->showMessage("A chill runs down my spine...", true);
      }
      incrShock(shockValue_heavy);
      eng->renderer->drawMapAndInterface();
    } else {
      if(eng->map->getDungeonLevel() != 0) {
        incrShock(1);
      }
    }
  }

  //Take sanity hit from high shock?
  if(getShockTotal() >= 100) {
    incrInsanity();
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
              eng->log->addMessage(actor->getNameA() + " comes into my view.", clrWhite, messageInterrupt_force);
            }
            monster->messageMonsterInViewPrinted = true;
          }
        } else {
          monster->messageMonsterInViewPrinted = false;

          //Is the monster sneaking? Try to spot it
          if(eng->map->playerVision[monster->pos.x][monster->pos.y]) {
            if(monster->isStealth) {
              const int PLAYER_SEARCH_SKILL = def_->abilityVals.getVal(ability_searching, true, *this);
              const AbilityRollResult_t rollResult = eng->abilityRoll->roll(PLAYER_SEARCH_SKILL);
              if(rollResult == successSmall) {
                eng->log->addMessage("I see something moving in the shadows.");
              } else if(rollResult > successSmall) {
                monster->isStealth = false;
                updateFov();
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

    if(eng->playerBonusHandler->isBonusPicked(playerBonus_rapidRecoverer)) {
      const int REGEN_N_TURN = 7;
      if((TURN / REGEN_N_TURN) * REGEN_N_TURN == TURN && TURN > 1) {
        if(getHp() < getHpMax(true)) {
          hp_++;
        }
      }
    }

    if(statusEffectsHandler_->allowSee() && statusEffectsHandler_->hasEffect(statusConfused) == false) {
      int x0 = pos.x - 1;
      int y0 = pos.y - 1;
      int x1 = pos.x + 1;
      int y1 = pos.y + 1;

      //Look for secret doors and traps
      for(int y = y0; y <= y1; y++) {
        for(int x = x0; x <= x1; x++) {
          if(eng->map->playerVision[x][y]) {
            Feature* f = eng->map->featuresStatic[x][y];

            if(f->getId() == feature_trap) {
              dynamic_cast<Trap*>(f)->playerTrySpotHidden();
            }
            if(f->getId() == feature_door) {
              dynamic_cast<Door*>(f)->playerTrySpotHidden();
            }
          }
        }
      }

      if(eng->playerBonusHandler->isBonusPicked(playerBonus_observant)) {
        const int CLUE_RADI = 3;
        x0 = max(0, pos.x - CLUE_RADI);
        y0 = max(0, pos.y - CLUE_RADI);
        x1 = min(MAP_X_CELLS - 1, pos.x + CLUE_RADI);
        y1 = max(MAP_Y_CELLS - 1, pos.y + CLUE_RADI);

        for(int y = y0; y <= y1; y++) {
          for(int x = x0; x <= x1; x++) {
            if(eng->map->playerVision[x][y]) {
              Feature* f = eng->map->featuresStatic[x][y];
              if(f->getId() == feature_door) {
                Door* door = dynamic_cast<Door*>(f);
                door->playerTryClueHidden();
              }
            }
          }
        }
      }

      //Any item in the inventory that can be identified?
      tryIdentifyItems();
    }
  }

  //First aid?
  if(firstAidTurnsLeft == 0) {
    eng->log->clearLog();
    eng->log->addMessage("I finish applying first aid.");
    if(eng->playerBonusHandler->isBonusPicked(playerBonus_curer)) {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(pos, visionBlockers);
      statusEffectsHandler_->endEffect(statusDiseased, visionBlockers);
    }
    restoreHP(99999);
    eng->renderer->drawMapAndInterface();
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

void Player::tryIdentifyItems() {
  //  const vector<Item*>* const general = inventory_->getGeneral();
  //  for(unsigned int i = 0; i < general->size(); i++) {
  //    Item* const item = general->at(i);
  //    const ItemDefinition& def = item->getInstanceDefinition();
  //
  //    //It must not be a readable item (those must be actively identified)
  //    if(def.isReadable == false) {
  //      if(def.isIdentified == false) {
  //        if(def.abilityToIdentify != ability_empty) {
  //          const int SKILL = m_instanceDefinition.abilityVals.getVal(def.abilityToIdentify, true);
  //          if(SKILL > (100 - def.identifySkillFactor)) {
  //            item->setRealDefinitionNames(eng, false);
  //            eng->log->addMessage("I recognize " + def.name.name_a + " in my inventory.", clrWhite, true);
  //          }
  //        }
  //      }
  //    }
  //  }
}

int Player::getHealingTimeTotal() const {
  const int TURNS_BEFORE_BON = 70;
  const int PLAYER_HEALING_RANK = eng->playerBonusHandler->isBonusPicked(playerBonus_skillfulWoundTreater);
  return PLAYER_HEALING_RANK >= 1 ? TURNS_BEFORE_BON / 2 : TURNS_BEFORE_BON;
}

void Player::interruptActions(const bool PROMPT_FOR_ABORT) {
  eng->renderer->drawMapAndInterface();

  eng->inventoryHandler->screenToOpenAfterDrop = endOfInventoryScreens;
  eng->inventoryHandler->browserPosToSetAfterDrop = 0;

  //Abort searching
  if(waitTurnsLeft > 0) {
    eng->log->addMessage("I stop waiting.", clrWhite);
    eng->renderer->drawMapAndInterface();
  }
  waitTurnsLeft = -1;

  const bool IS_FAINTED = statusEffectsHandler_->hasEffect(statusFainted);
  const bool IS_PARALYSED = statusEffectsHandler_->hasEffect(statusParalyzed);
  const bool IS_DEAD = deadState != actorDeadState_alive;

  //If monster is in view, or player is paralysed, fainted or dead, abort first aid - else query abort
  if(firstAidTurnsLeft > 0) {
    getSpotedEnemies();
    const int TOTAL_TURNS = getHealingTimeTotal();
    const bool IS_ENOUGH_TIME_PASSED = firstAidTurnsLeft < TOTAL_TURNS - 10;
    const int MISSING_HP = getHpMax(true) - getHp();
    const int HP_HEALED_IF_ABORTED = IS_ENOUGH_TIME_PASSED ? (MISSING_HP * (TOTAL_TURNS - firstAidTurnsLeft)) / TOTAL_TURNS  : 0;

    bool isAborted = false;
    if(spotedEnemies.size() > 0 || IS_FAINTED || IS_PARALYSED || IS_DEAD || PROMPT_FOR_ABORT == false) {
      firstAidTurnsLeft = -1;
      isAborted = true;
      eng->log->addMessage("I stop tending to my wounds.", clrWhite);
      eng->renderer->drawMapAndInterface();
    } else {
      const string TURNS_STR = intToString(firstAidTurnsLeft);
      const string ABORTED_HP_STR = intToString(HP_HEALED_IF_ABORTED);
      string abortStr = "Continue healing (" + TURNS_STR + " turns)? (y/n), ";
      abortStr += ABORTED_HP_STR + " HP restored if canceled.";
      eng->log->addMessage(abortStr , clrWhiteHigh);
      eng->renderer->drawMapAndInterface();

      if(eng->query->yesOrNo() == false) {
        firstAidTurnsLeft = -1;
        isAborted = true;
      }

      eng->log->clearLog();
      eng->renderer->drawMapAndInterface();
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

void Player::hearSound(const Sound& sound) {
  const string message = sound.getMessage();
  if(message != "") {
    eng->log->addMessage(message);
  }
}

void Player::moveDirection(const int X_DIR, const int Y_DIR) {
  if(deadState == actorDeadState_alive) {
    coord dest = statusEffectsHandler_->changeMoveCoord(pos, pos + coord(X_DIR, Y_DIR));

    //Trap affects leaving?
    if(dest != pos) {
      Feature* f = eng->map->featuresStatic[pos.x][pos.y];
      if(f->getId() == feature_trap) {
        tracer << "Player: Standing on trap, check if trap affects leaving the cell" << endl;
        dest = dynamic_cast<Trap*>(f)->actorTryLeave(this, pos, dest);
      }
    }

    bool isFreeTurn = false;;

    if(dest != pos) {
      //Attack?
      Actor* const actorAtDest = eng->mapTests->getActorAtPos(dest);
      if(actorAtDest != NULL) {
        if(statusEffectsHandler_->allowAttackMelee(true) == true) {
          bool hasMeleeWeapon = false;
          Weapon* weapon = dynamic_cast<Weapon*>(inventory_->getItemInSlot(slot_wielded));
          if(weapon != NULL) {
            if(weapon->getDef().isMeleeWeapon) {
              if(eng->config->RANGED_WPN_MELEE_PROMPT && checkIfSeeActor(*actorAtDest, NULL)) {
                if(weapon->getDef().isRangedWeapon) {
                  const string wpnName = eng->itemData->getItemRef(weapon, itemRef_a);
                  eng->log->addMessage("Attack " + actorAtDest->getNameThe() + " with " + wpnName + "? (y/n)", clrWhiteHigh);
                  eng->renderer->drawMapAndInterface();
                  if(eng->query->yesOrNo() == false) {
                    eng->log->clearLog();
                    eng->renderer->drawMapAndInterface();
                    return;
                  }
                }
              }
              hasMeleeWeapon = true;
              eng->attack->melee(dest, weapon);
              target = actorAtDest;
              return;
            }
          }
          if(hasMeleeWeapon == false) {
            punch(*actorAtDest);
          }
        }
        return;
      }

      //This point reached means no actor in the destination cell.

      //Blocking mobile or static features?
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
        if(inventory_->getTotalItemWeight() >= getCarryWeightLimit()) {
          eng->log->addMessage("I am too encumbered to move!");
          eng->renderer->drawMapAndInterface();
          return;
        }

        pos = dest;

        const bool IS_DEXTEROUS_PICKED = eng->playerBonusHandler->isBonusPicked(playerBonus_dexterous);
        const bool IS_LITHE_PICKED = eng->playerBonusHandler->isBonusPicked(playerBonus_lithe);
        const bool IS_MOBILE_PICKED = eng->playerBonusHandler->isBonusPicked(playerBonus_mobile);
        if(IS_MOBILE_PICKED || IS_DEXTEROUS_PICKED || IS_LITHE_PICKED) {
          const int FREE_MOVE_EVERY_N_TURN = IS_MOBILE_PICKED ? 2 : (IS_LITHE_PICKED ? 4 : 5);
          if(nrMovesUntilFreeAction == -1) {
            nrMovesUntilFreeAction = FREE_MOVE_EVERY_N_TURN - 2;
          } else if(nrMovesUntilFreeAction == 0) {
            nrMovesUntilFreeAction = FREE_MOVE_EVERY_N_TURN - 1;
            isFreeTurn = true;
          } else {
            nrMovesUntilFreeAction--;
          }
        }

        //Print message if walking on item
        Item* const item = eng->map->items[pos.x][pos.y];
        if(item != NULL) {
          string message = statusEffectsHandler_->allowSee() == false ? "I feel here: " : "I see here: ";
          message += eng->itemData->getItemInterfaceRef(item, true);
          eng->log->addMessage(message + ".");
        }
      }

      //Note: bump() prints block messages.
      for(unsigned int i = 0; i < featureMobs.size(); i++) {
        featureMobs.at(i)->bump(this);
      }
      eng->map->featuresStatic[dest.x][dest.y]->bump(this);
    }
    //If destination reached, then we either moved or were held by something.
    //End turn (unless free turn due to bonus).
    if(pos == dest) {
      if(isFreeTurn == false) {
        eng->gameTime->letNextAct();
        return;
      }
    }
    eng->gameTime->updateLightMap();
    updateFov();
    eng->renderer->drawMapAndInterface();
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

void Player::kick(Actor& actorToKick) {
  Weapon* kickWeapon = NULL;

  const ActorDefinition* const d = actorToKick.getDef();

  if(d->actorSize == actorSize_floor && (d->isSpider == true || d->isRat == true)) {
    kickWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerStomp));
  } else {
    kickWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerKick));
  }
  eng->attack->melee(actorToKick.pos, kickWeapon);
  delete kickWeapon;
}

void Player::punch(Actor& actorToPunch) {
  //Spawn a temporary punch weapon to attack with
  Weapon* punchWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerPunch));
  eng->attack->melee(actorToPunch.pos, punchWeapon);
  delete punchWeapon;
}

void Player::actorSpecific_addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {

  bool isUsingLightGivingItem = flareFuseTurns > 0;

  if(isUsingLightGivingItem == false) {
    for(unsigned int i = 0; i < inventory_->getGeneral()->size(); i++) {
      Item* const item = inventory_->getGeneral()->at(i);
      if(item->getDef().id == item_deviceElectricLantern) {
        DeviceElectricLantern* const lantern = dynamic_cast<DeviceElectricLantern*>(item);
        isUsingLightGivingItem = lantern->isGivingLight();
        break;
      }
    }
  }

  if(isUsingLightGivingItem) {
    bool myLight[MAP_X_CELLS][MAP_Y_CELLS];
    eng->basicUtils->resetBoolArray(myLight, false);
    const int RADI = FOV_STANDARD_RADI_INT; //LitFlare::getLightRadius();
    coord x0y0(max(0, pos.x - RADI), max(0, pos.y - RADI));
    coord x1y1(min(MAP_X_CELLS - 1, pos.x + RADI), min(MAP_Y_CELLS - 1, pos.y + RADI));

    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      for(int x = x0y0.x; x <= x1y1.x; x++) {
        visionBlockers[x][y] = !eng->map->featuresStatic[x][y]->isVisionPassable();
      }
    }

    eng->fov->runFovOnArray(visionBlockers, pos, myLight, false);
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      for(int x = x0y0.x; x <= x1y1.x; x++) {
        if(myLight[x][y]) {
          light[x][y] = true;
        }
      }
    }
  }
}

void Player::updateFov() {
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();

  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    eng->gameTime->getFeatureMobAt(i)->addLight(eng->map->light);
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->map->featuresStatic[x][y]->addLight(eng->map->light);
      eng->map->playerVision[x][y] = false;
    }
  }

  if(statusEffectsHandler_->allowSee()) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeVisionBlockerArray(pos, blockers);
    eng->fov->runPlayerFov(blockers, pos);
    eng->map->playerVision[pos.x][pos.y] = true;
  }

  if(statusEffectsHandler_->hasEffect(statusClairvoyant)) {
    const int FLOODFILL_TRAVEL_LIMIT = FOV_STANDARD_RADI_INT + 2;

    const int X0 = max(0, pos.x - FLOODFILL_TRAVEL_LIMIT);
    const int Y0 = max(0, pos.y - FLOODFILL_TRAVEL_LIMIT);
    const int X1 = min(MAP_X_CELLS - 1, pos.x + FLOODFILL_TRAVEL_LIMIT);
    const int Y1 = min(MAP_Y_CELLS - 1, pos.y + FLOODFILL_TRAVEL_LIMIT);

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_fly, blockers);

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
          blockers[x][y] = false;
        }
      }
    }

    int floodFillValues[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeFloodFill(pos, blockers, floodFillValues, FLOODFILL_TRAVEL_LIMIT, coord(-1, -1));

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(floodFillValues[x][y]) {
          eng->map->playerVision[x][y] = true;
        }
      }
    }
  }

  if(statusEffectsHandler_->allowSee()) {
    FOVhack();
  }

  if(eng->isCheatVisionEnabled) {
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
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(pos, visionBlockers, 9999);

  bool moveBlocked[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayFeaturesOnly(eng->player, moveBlocked);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(visionBlockers[x][y] && moveBlocked[x][y]) {
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

void Player::grantMthPower() const {
  if(eng->itemData->itemDefinitions[item_thaumaturgicAlteration]->isScrollLearned == false) {
    eng->itemData->itemDefinitions[item_thaumaturgicAlteration]->isScrollLearned = true;
    string str = "I have gained a deeper insight into the esoteric forces acting behind our apparent reality.";
    str += " With this knowledge, I can attempt to acquire hidden information or displace existence according to my will.";
    str += " Gained spell: Thaumaturgic Alteration";
    eng->popup->showMessage(str, true, "Thaumaturgic Alteration");
  }
}



