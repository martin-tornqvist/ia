#include "ActorPlayer.h"

#include "Engine.h"

#include "Renderer.h"
#include "Audio.h"
#include "ItemWeapon.h"
#include "FeatureTrap.h"
#include "PlayerCreateCharacter.h"
#include "Log.h"
#include "CharacterLines.h"
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
#include "Inventory.h"
#include "InventoryHandler.h"
#include "ItemMedicalBag.h"
#include "PlayerSpellsHandler.h"

Player::Player() :
  activeMedicalBag(NULL), waitTurnsLeft(-1), dynamiteFuseTurns(-1),
  molotovFuseTurns(-1), flareFuseTurns(-1),
  target(NULL), insanity_(0), shock_(0.0), shockTemp_(0.0),
  mth(0), nrMovesUntilFreeAction(-1), carryWeightBase(450) {
}

void Player::actorSpecific_spawnStartItems() {
  data_->abilityVals.reset();

  for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
    insanityPhobias[i] = false;
  }
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    insanityObsessions[i] = false;
  }

  int NR_CARTRIDGES        = eng->dice.range(1, 2);
  int NR_DYNAMITE          = eng->dice.range(2, 3);
  int NR_MOLOTOV           = eng->dice.range(2, 3);
//  int NR_FLARES            = eng->dice.range(1, 6);
  int NR_THROWING_KNIVES   = eng->dice.range(7, 12);
  int NR_SPIKES            = 0; //eng->dice.coinToss() ? 0 : eng->dice.range(3, 4);

  const int WEAPON_CHOICE = eng->dice.range(1, 5);
  ItemId_t weaponId = item_dagger;
  switch(WEAPON_CHOICE) {
    case 1:   weaponId = item_dagger;   break;
    case 2:   weaponId = item_hatchet;  break;
    case 3:   weaponId = item_hammer;   break;
    case 4:   weaponId = item_machete;  break;
    case 5:   weaponId = item_axe;      break;
    default:  weaponId = item_dagger;   break;
  }

  inventory_->putItemInSlot(
    slot_wielded, eng->itemFactory->spawnItem(weaponId), true, true);

  inventory_->putItemInSlot(
    slot_wieldedAlt, eng->itemFactory->spawnItem(item_pistol), true, true);

  for(int i = 0; i < NR_CARTRIDGES; i++) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
  }

  //TODO Remove:
  //--------------------------------------------------------------------------
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_machineGun));
  for(int i = 0; i < 2; i++) {
    inventory_->putItemInGeneral(
      eng->itemFactory->spawnItem(item_drumOfBullets));
  }
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_sawedOff));
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_pumpShotgun));
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_shotgunShell, 80));
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_armorAsbestosSuit));
  //--------------------------------------------------------------------------


  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_dynamite, NR_DYNAMITE));
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_molotov, NR_MOLOTOV));

  if(NR_THROWING_KNIVES > 0) {
    inventory_->putItemInSlot(
      slot_missiles,
      eng->itemFactory->spawnItem(item_throwingKnife, NR_THROWING_KNIVES),
      true, true);
  }

  if(NR_SPIKES > 0) {
    inventory_->putItemInGeneral(
      eng->itemFactory->spawnItem(item_ironSpike, NR_SPIKES));
  }

  inventory_->putItemInSlot(
    slot_armorBody,
    eng->itemFactory->spawnItem(item_armorLeatherJacket),
    true, true);

  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_deviceElectricLantern));
  inventory_->putItemInGeneral(
    eng->itemFactory->spawnItem(item_medicalBag));

//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceSentry));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceRejuvenator));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceRepeller));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_deviceTranslocator));
//  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_scrollOfIdentify, 4));
}

void Player::addSaveLines(vector<string>& lines) const {
  const int NR_STATUS_EFFECTS = propHandler_->appliedProps_.size();
  lines.push_back(toString(NR_STATUS_EFFECTS));
  for(int i = 0; i < NR_STATUS_EFFECTS; i++) {
    lines.push_back(
      toString(propHandler_->appliedProps_.at(i)->getId()));
    lines.push_back(
      toString(propHandler_->appliedProps_.at(i)->getId()));
  }

  lines.push_back(toString(insanity_));
  lines.push_back(toString(int(shock_)));
  lines.push_back(toString(mth));
  lines.push_back(toString(hp_));
  lines.push_back(toString(hpMax_));
  lines.push_back(toString(spi_));
  lines.push_back(toString(spiMax_));
  lines.push_back(toString(pos.x));
  lines.push_back(toString(pos.y));
  lines.push_back(toString(dynamiteFuseTurns));
  lines.push_back(toString(molotovFuseTurns));
  lines.push_back(toString(flareFuseTurns));

  for(int i = 0; i < endOfAbilities; i++) {
    lines.push_back(toString(data_->abilityVals.getRawVal(Abilities_t(i))));
  }

  for(int i = 0; i < endOfInsanityPhobias; i++) {
    lines.push_back(insanityPhobias[i] == 0 ? "0" : "1");
  }
  for(int i = 0; i < endOfInsanityObsessions; i++) {
    lines.push_back(insanityObsessions[i] == 0 ? "0" : "1");
  }
}

void Player::setParametersFromSaveLines(vector<string>& lines) {
  const int NR_STATUS_EFFECTS = toInt(lines.front());
  lines.erase(lines.begin());
  for(int i = 0; i < NR_STATUS_EFFECTS; i++) {
    const PropId_t id = (PropId_t)(toInt(lines.front()));
    lines.erase(lines.begin());
    const int TURNS = toInt(lines.front());
    lines.erase(lines.begin());
    Prop* const prop =
      propHandler_->makePropFromId(id, propTurnsSpecified, TURNS);
    propHandler_->tryApplyProp(prop, true, true, true);
  }

  insanity_ = toInt(lines.front());
  lines.erase(lines.begin());
  shock_ = double(toInt(lines.front()));
  lines.erase(lines.begin());
  mth = toInt(lines.front());
  lines.erase(lines.begin());
  hp_ = toInt(lines.front());
  lines.erase(lines.begin());
  hpMax_ = toInt(lines.front());
  lines.erase(lines.begin());
  spi_ = toInt(lines.front());
  lines.erase(lines.begin());
  spiMax_ = toInt(lines.front());
  lines.erase(lines.begin());
  pos.x = toInt(lines.front());
  lines.erase(lines.begin());
  pos.y = toInt(lines.front());
  lines.erase(lines.begin());
  dynamiteFuseTurns = toInt(lines.front());
  lines.erase(lines.begin());
  molotovFuseTurns = toInt(lines.front());
  lines.erase(lines.begin());
  flareFuseTurns = toInt(lines.front());
  lines.erase(lines.begin());

  for(int i = 0; i < endOfAbilities; i++) {
    data_->abilityVals.setVal(Abilities_t(i), toInt(lines.front()));
    lines.erase(lines.begin());
  }

  for(int i = 0; i < endOfInsanityPhobias; i++) {
    insanityPhobias[i] = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());
  }
  for(int i = 0; i < endOfInsanityObsessions; i++) {
    insanityObsessions[i] = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());
  }
}

void Player::actorSpecific_hit(const int DMG, const bool ALLOW_WOUNDS) {
  //Hit aborts first aid
  if(activeMedicalBag != NULL) {
    activeMedicalBag->interrupted();
    activeMedicalBag = NULL;
  }

  if(insanityObsessions[insanityObsession_masochism]) {
    if(DMG > 1) {
      shock_ = max(0.0, shock_ - 5.0);
    }
  } else {
    incrShock(1);
  }

  if(ALLOW_WOUNDS && eng->config->isBotPlaying == false) {
    if(DMG >= 5) {
      Prop* const prop = new PropWound(eng, propTurnsIndefinite);
      propHandler_->tryApplyProp(prop);
    }
  }

  eng->renderer->drawMapAndInterface();
}

int Player::getCarryWeightLimit() const {
  PlayerBonHandler* const bon = eng->playerBonHandler;
  const bool IS_TOUGH         = bon->isBonPicked(playerBon_tough);
  const bool IS_STRONG_BACKED = bon->isBonPicked(playerBon_strongBacked);
  const bool IS_WEAKENED      = propHandler_->hasProp(propWeakened);
  const int CARRY_WEIGHT_MOD =
    IS_TOUGH * 10 + IS_STRONG_BACKED * 30 - IS_WEAKENED * 15;

  return (carryWeightBase * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::getShockResistance() const {
  int ret = 0;
  if(eng->playerBonHandler->isBonPicked(playerBon_fearless)) {
    ret += 5;
  }
  if(eng->playerBonHandler->isBonPicked(playerBon_coolHeaded)) {
    ret += 20;
  }
  return min(100, max(0, ret));
}

void Player::incrShock(const int VAL) {
  const double SHOCK_RES_DB = double(getShockResistance());
  const double VAL_DB = double(VAL);
  const double VAL_AFTER_SHOCK_RES =
    (VAL_DB * (100.0 - SHOCK_RES_DB)) / 100.0;
  shock_ = min(100.0, shock_ + max(0.0, VAL_AFTER_SHOCK_RES));
}

void Player::incrShock(const ShockValues_t shockValue) {
  switch(shockValue) {
    case shockValue_none:   {incrShock(0);} break;
    case shockValue_mild:   {incrShock(2);} break;
    case shockValue_some:   {incrShock(4);} break;
    case shockValue_heavy:  {incrShock(12);} break;
    default: {} break;
  }
}

void Player::restoreShock(const int amountRestored,
                          const bool IS_TEMP_SHOCK_RESTORED) {
  // If an obsession is active, only restore to a certain min level
  bool isObsessionActive = 0;
  for(int i = 0; i < endOfInsanityObsessions; i++) {
    if(insanityObsessions[i]) {
      isObsessionActive = true;
      break;
    }
  }
  const double MIN_SHOCK_WHEN_OBSESSION_DB =
    double(MIN_SHOCK_WHEN_OBSESSION);
  shock_ = max(
             (isObsessionActive ? MIN_SHOCK_WHEN_OBSESSION_DB : 0.0),
             shock_ - amountRestored);
  shockTemp_ = IS_TEMP_SHOCK_RESTORED ? 0 : shockTemp_;
}

void Player::incrMth(const int VAL) {
  mth = max(0, min(100, mth + VAL));
  insanity_ = max(0, min(100, insanity_ + VAL / 2));
}

void Player::incrInsanity() {
  string popupMessage = getInsanity() < 100 ? "Insanity draws nearer... " : "";

  const int INS_INCR = 6;

  if(eng->config->isBotPlaying == false) {
    insanity_ += INS_INCR;
  }

  mth = min(100, mth + INS_INCR / 2);

  restoreShock(70, false);

  updateColor();
  eng->renderer->drawMapAndInterface();

  if(getInsanity() >= 100) {
    popupMessage += "My mind can no longer withstand what it has grasped.";
    popupMessage += " I am hopelessly lost.";
    eng->popup->showMessage(popupMessage, true, "Complete insanity!");
    die(true, false, false);
  } else {
    bool playerSeeShockingMonster = false;
    vector<Actor*> spotedEnemies;
    getSpotedEnemies(spotedEnemies);
    for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
      const ActorData* const def = spotedEnemies.at(i)->getData();
      if(def->monsterShockLevel != monsterShockLevel_none) {
        playerSeeShockingMonster = true;
      }
    }

    //When long term sanity decreases something happens (mostly bad)
    //(Reroll until something actually happens)
    while(true) {
      const int ROLL = eng->dice.range(1, 8);
      switch(ROLL) {
        case 1: {
          if(playerSeeShockingMonster) {
            if(eng->dice.coinToss()) {
              popupMessage += "I let out a terrified shriek.";
            } else {
              popupMessage += "I scream in terror.";
            }
            eng->popup->showMessage(popupMessage, true, "Screaming!");
            eng->soundEmitter->emitSound(Sound("", endOfSfx, true, pos, true, true));
            return;
          }
        } break;

        case 2: {
          popupMessage += "I find myself babbling incoherently.";
          eng->popup->showMessage(popupMessage, true, "Babbling!");
          const string playerName = getNameThe();
          for(int i = eng->dice.range(3, 5); i > 0; i--) {
            const string phrase = Cultist::getCultistPhrase(eng);
            eng->log->addMsg(playerName + ": " + phrase);
          }
          eng->soundEmitter->emitSound(Sound("", endOfSfx, true, pos, false, true));
          return;
        } break;

        case 3: {
          popupMessage += "I struggle to not fall into a stupor.";
          eng->popup->showMessage(popupMessage, true, "Fainting!");
          propHandler_->tryApplyProp(
            new PropFainted(eng, propTurnsStandard));
          return;
        } break;

        case 4: {
          popupMessage += "I laugh nervously.";
          eng->popup->showMessage(popupMessage, true, "HAHAHA!");
          eng->soundEmitter->emitSound(
            Sound("", endOfSfx, true, pos, false, true));
          return;
        } break;

        case 5: {
          if(eng->player->getPropHandler()->hasProp(propRFear) == false) {

            if(insanity_ > 5) {
              //There is a limit to the number of phobias you can have
              int phobiasActive = 0;
              for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
                if(insanityPhobias[i]) {
                  phobiasActive++;
                }
              }
              if(phobiasActive < 2) {
                if(eng->dice.coinToss()) {
                  if(spotedEnemies.size() > 0) {
                    const int MONSTER_ROLL = eng->dice(1, spotedEnemies.size()) - 1;
                    const ActorData* const monsterData =
                      spotedEnemies.at(MONSTER_ROLL)->getData();
                    if(
                      monsterData->isRat &&
                      insanityPhobias[insanityPhobia_rat] == false) {
                      popupMessage += "I am afflicted by Murophobia. Rats suddenly seem terrifying.";
                      eng->popup->showMessage(popupMessage, true, "Murophobia!");
                      insanityPhobias[insanityPhobia_rat] = true;
                      return;
                    }
                    if(
                      monsterData->isSpider &&
                      insanityPhobias[insanityPhobia_spider] == false) {
                      popupMessage += "I am afflicted by Arachnophobia. Spiders suddenly seem terrifying.";
                      eng->popup->showMessage(popupMessage, true, "Arachnophobia!");
                      insanityPhobias[insanityPhobia_spider] = true;
                      return;
                    }
                    if(
                      monsterData->isCanine &&
                      insanityPhobias[insanityPhobia_dog] == false) {
                      popupMessage += "I am afflicted by Cynophobia. Dogs suddenly seem terrifying.";
                      eng->popup->showMessage(popupMessage, true, "Cynophobia!");
                      insanityPhobias[insanityPhobia_dog] = true;
                      return;
                    }
                    if(
                      monsterData->isUndead &&
                      insanityPhobias[insanityPhobia_undead] == false) {
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
                    if(eng->map->getDLVL() >= 5) {
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
        } break;

        case 6: {
          if(insanity_ > 20) {
            int obsessionsActive = 0;
            for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
              if(insanityObsessions[i]) {
                obsessionsActive++;
              }
            }
            if(obsessionsActive == 0) {
              const InsanityObsession_t obsession =
                (InsanityObsession_t)(eng->dice.range(
                                        0, endOfInsanityObsessions - 1));
              switch(obsession) {
                case insanityObsession_masochism: {
                  popupMessage += "To my alarm, I find myself encouraged by the sensation of pain. Every time I am hurt, ";
                  popupMessage += "I find a little relief. However, my depraved mind can no longer find complete peace ";
                  popupMessage += "(shock cannot go below " + toString(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  eng->popup->showMessage(popupMessage, true, "Masochistic obsession!");
                  insanityObsessions[insanityObsession_masochism] = true;
                  return;
                } break;
                case insanityObsession_sadism: {
                  popupMessage += "To my alarm, I find myself encouraged by the pain I cause in others. For every life I take, ";
                  popupMessage += "I find a little relief. However, my depraved mind can no longer find complete peace ";
                  popupMessage += "(shock cannot go below " + toString(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  eng->popup->showMessage(popupMessage, true, "Sadistic obsession!");
                  insanityObsessions[insanityObsession_sadism] = true;
                  return;
                } break;
                default: {} break;
              }
            }
          }
        } break;

        case 7: {
          if(insanity_ > 8) {
            popupMessage += "The shadows are closing in on me!";
            eng->popup->showMessage(popupMessage, true, "Haunted by shadows!");

            bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
            eng->mapTests->makeMoveBlockerArrayForBodyType(
              actorBodyType_normal, blockers);

            vector<Pos> spawnPosCandidates;

            const int D_MAX = FOV_STANDARD_RADI_INT - 1;
            for(int dx = -D_MAX; dx <= D_MAX; dx++) {
              for(int dy = -D_MAX; dy <= D_MAX; dy++) {
                if(dx <= -2 || dx >= 2 || dy <= -2 || dy >= 2) {
                  if(blockers[pos.x + dx][pos.y + dy] == false) {
                    spawnPosCandidates.push_back(pos + Pos(dx, dy));
                  }
                }
              }
            }
            const int NR_SPAWN_POS_CANDIDATES = spawnPosCandidates.size();
            if(NR_SPAWN_POS_CANDIDATES > 0) {
              const int NR_SHADOWS_LOWER = 2;
              const int NR_SHADOWS_UPPER =
                max(2, min(6, (eng->map->getDLVL() + 1) / 2));
              const int NR_SHADOWS_TO_SPAWN =
                min(NR_SPAWN_POS_CANDIDATES,
                    eng->dice.range(NR_SHADOWS_LOWER, NR_SHADOWS_UPPER));
              for(int i = 0; i < NR_SHADOWS_TO_SPAWN; i++) {
                const unsigned int SPAWN_POS_ELEMENT =
                  eng->dice.range(0, spawnPosCandidates.size() - 1);
                const Pos spawnPos = spawnPosCandidates.at(SPAWN_POS_ELEMENT);
                spawnPosCandidates.erase(
                  spawnPosCandidates.begin() + SPAWN_POS_ELEMENT);
                Monster* monster =
                  dynamic_cast<Monster*>(
                    eng->actorFactory->spawnActor(actor_shadow, spawnPos));
                monster->playerAwarenessCounter =
                  monster->getData()->nrTurnsAwarePlayer;
                if(eng->dice.coinToss()) {
                  monster->isStealth = true;
                }
              }
            }
            return;
          }
        } break;

        case 8: {
          popupMessage += "I find myself in a peculiar detached daze, ";
          popupMessage += "a tranced state of mind. I struggle to recall ";
          popupMessage += "where I am, or what I'm doing.";
          eng->popup->showMessage(popupMessage, true, "Confusion!");

          propHandler_->tryApplyProp(
            new PropConfused(eng, propTurnsStandard));

          return;
        } break;

        default: {} break;
      }
    }
  }
}

void Player::setTempShockFromFeatures() {
  if(
    eng->map->darkness[pos.x][pos.y] &&
    eng->map->light[pos.x][pos.y] == false) {
    shockTemp_ += 20;
  }

  for(int dy = -1; dy <= 1; dy++) {
    const int Y = pos.y + dy;
    for(int dx = -1; dx <= 1; dx++) {
      const int X = pos.x + dx;
      if(eng->mapTests->isCellInsideMap(Pos(X, Y))) {
        const Feature* const f =
          eng->map->featuresStatic[pos.x + dx][pos.y + dy];
        shockTemp_ += f->getShockWhenAdjacent();
      }
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
  vector<Actor*> spotedEnemies;
  getSpotedEnemies(spotedEnemies);

  const int ROLL = eng->dice.percentile();
  //Phobia vs creature type?
  if(ROLL < 10) {
    for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
      const ActorData* const monsterData = spotedEnemies.at(0)->getData();
      if(
        monsterData->isCanine &&
        insanityPhobias[insanityPhobia_dog]) {
        eng->log->addMsg("I am plagued by my canine phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng->dice(1, 6)));
        return;
      }
      if(
        monsterData->isRat &&
        insanityPhobias[insanityPhobia_rat]) {
        eng->log->addMsg("I am plagued by my rat phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng->dice(1, 6)));
        return;
      }
      if(
        monsterData->isUndead &&
        insanityPhobias[insanityPhobia_undead]) {
        eng->log->addMsg("I am plagued by my phobia of the dead!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng->dice(1, 6)));
        return;
      }
      if(
        monsterData->isSpider &&
        insanityPhobias[insanityPhobia_spider]) {
        eng->log->addMsg("I am plagued by my spider phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng->dice(1, 6)));
        return;
      }
    }
  }
  if(ROLL < 5) {
    if(insanityPhobias[insanityPhobia_openPlace]) {
      if(isStandingInOpenSpace()) {
        eng->log->addMsg("I am plagued by my phobia of open places!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng->dice(1, 6)));
        return;
      }
    }

    if(insanityPhobias[insanityPhobia_closedPlace]) {
      if(isStandingInCrampedSpace()) {
        eng->log->addMsg("I am plagued by my phobia of closed places!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng->dice(1, 6)));
        return;
      }
    }
  }
}

int Player::getHpMax(const bool WITH_MODIFIERS) const {
  if(WITH_MODIFIERS) {
    if(propHandler_->hasProp(propDiseased)) {
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

  if(propHandler_->changeActorClr(clr_)) {
    return;
  }

  if(dynamiteFuseTurns > 0 || molotovFuseTurns > 0 || flareFuseTurns > 0) {
    clr_ = clrYellow;
    return;
  }

  const int CUR_SHOCK = shock_ + shockTemp_;
  if(CUR_SHOCK >= 75) {
    clr_ = clrMagenta;
    return;
  }

  clr_ = data_->color;
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
      eng->log->addMsg(fuseMessage, clrYellow);
    }
  }
  if(dynamiteFuseTurns == 0) {
    eng->log->addMsg("The dynamite explodes in my hands!");
    eng->explosionMaker->runExplosion(pos);
    updateColor();
    dynamiteFuseTurns = -1;
  }

  //Molotovs
  if(molotovFuseTurns > 0) {
    molotovFuseTurns--;
  }
  if(molotovFuseTurns == 0) {
    eng->log->addMsg("The Molotov Cocktail explodes in my hands!");
    molotovFuseTurns = -1;
    updateColor();
    eng->explosionMaker->runExplosion(
      pos, sfxExplosionMolotov, false,
      new PropBurning(eng, propTurnsStandard));
  }

  //Flare
  if(flareFuseTurns > 0) {
    flareFuseTurns--;
  }
  if(flareFuseTurns == 0) {
    eng->log->addMsg("The flare is extinguished.");
    updateColor();
    flareFuseTurns = -1;
  }

  if(activeMedicalBag == NULL) {
    testPhobias();
  }

  //If obsessions are active, raise shock to a minimum level
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    if(insanityObsessions[i]) {
      shock_ = max(double(MIN_SHOCK_WHEN_OBSESSION), shock_);
      break;
    }
  }

  //Shock from seen monsters
  vector<Actor*> spotedEnemies;
  getSpotedEnemies(spotedEnemies);
  double shockFromMonstersCurrentPlayerTurn = 0.0;
  for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
    Monster* monster = dynamic_cast<Monster*>(spotedEnemies.at(i));
    const ActorData* const data = monster->getData();
    if(data->monsterShockLevel != monsterShockLevel_none) {
      switch(data->monsterShockLevel) {
        case monsterShockLevel_unsettling: {
          monster->shockCausedCurrent += 0.10;
          monster->shockCausedCurrent =
            min(monster->shockCausedCurrent + 0.05, 1.0);
        } break;
        case monsterShockLevel_scary: {
          monster->shockCausedCurrent =
            min(monster->shockCausedCurrent + 0.15, 1.0);
        } break;
        case monsterShockLevel_terrifying: {
          monster->shockCausedCurrent =
            min(monster->shockCausedCurrent + 0.5, 2.0);
        } break;
        case monsterShockLevel_mindShattering: {
          monster->shockCausedCurrent =
            min(monster->shockCausedCurrent + 0.75, 3.0);
        } break;
        default: {} break;
      }
      if(shockFromMonstersCurrentPlayerTurn < 3.0) {
        incrShock(int(floor(monster->shockCausedCurrent)));
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
      if(eng->map->getDLVL() != 0) {
        incrShock(1);
      }
    }
  }

  //Take sanity hit from high shock?
  if(getShockTotal() >= 100) {
    incrInsanity();
    eng->gameTime->endTurnOfCurrentActor();
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
            if(activeMedicalBag != NULL || waitTurnsLeft > 0) {
              eng->log->addMsg(actor->getNameA() + " comes into my view.",
                               clrWhite, true);
            }
            monster->messageMonsterInViewPrinted = true;
          }
        } else {
          monster->messageMonsterInViewPrinted = false;

          //Is the monster sneaking? Try to spot it
          if(eng->map->playerVision[monster->pos.x][monster->pos.y]) {
            if(monster->isStealth) {
              const int PLAYER_SEARCH_SKILL =
                data_->abilityVals.getVal(ability_searching, true, *this);
              const AbilityRollResult_t rollResult =
                eng->abilityRoll->roll(PLAYER_SEARCH_SKILL);
              if(rollResult == successSmall) {
                eng->log->addMsg("I see something moving in the shadows.");
              } else if(rollResult > successSmall) {
                monster->isStealth = false;
                updateFov();
                eng->renderer->drawMapAndInterface();
                eng->log->addMsg("I spot " + monster->getNameA() + "!");
              }
            }
          }
        }
      }
    }
  }

  if(getHp() > getHpMax(true)) {
    hp_--;
  }
  if(getSpi() > getSpiMax()) {
    spi_--;
  }

  if(activeMedicalBag == NULL) {
    if(propHandler_->hasProp(propPoisoned) == false) {
      int nrWounds = 0;
      Prop* const propWnd = propHandler_->getAppliedProp(propWound);
      if(propWnd != NULL) {
        nrWounds = dynamic_cast<PropWound*>(propWnd)->getNrWounds();
      }

      const bool IS_RAPID_REC =
        eng->playerBonHandler->isBonPicked(playerBon_rapidRecoverer);

      const int REGEN_N_TURN = (IS_RAPID_REC ? 6 : 10) + (nrWounds * 5);

      if((TURN / REGEN_N_TURN) * REGEN_N_TURN == TURN && TURN > 1) {
        if(getHp() < getHpMax(true)) {
          hp_++;
        }
      }
    }

    if(
      propHandler_->allowSee() &&
      propHandler_->hasProp(propConfused) == false) {
      int x0 = pos.x - 1;
      int y0 = pos.y - 1;
      int x1 = pos.x + 1;
      int y1 = pos.y + 1;

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

      if(eng->playerBonHandler->isBonPicked(playerBon_observant)) {
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
    }
  }

  if(activeMedicalBag != NULL) {
    eng->renderer->drawMapAndInterface();
    eng->sleep(DELAY_PLAYER_WAITING);
    activeMedicalBag->continueAction();
  }

  if(waitTurnsLeft > 0) {
    eng->renderer->drawMapAndInterface();
    eng->sleep(DELAY_PLAYER_WAITING);
    waitTurnsLeft--;
    eng->gameTime->endTurnOfCurrentActor();
  }

  //When this function ends, the system starts reading keys.
}

//void Player::tryIdentifyItems() {
//  const vector<Item*>* const general = inventory_->getGeneral();
//  for(unsigned int i = 0; i < general->size(); i++) {
//    Item* const item = general->at(i);
//    const ItemDef& def = item->getInstanceDefinition();
//
//    //It must not be a readable item (those must be actively identified)
//    if(def.isScroll == false) {
//      if(def.isIdentified == false) {
//        if(def.abilityToIdentify != ability_empty) {
//          const int SKILL = m_instanceDefinition.abilityVals.getVal(def.abilityToIdentify, true);
//          if(SKILL > (100 - def.identifySkillFactor)) {
//            item->setRealDefinitionNames(eng, false);
//            eng->log->addMsg("I recognize " + def.name.name_a + " in my inventory.", clrWhite, true);
//          }
//        }
//      }
//    }
//  }
//}

void Player::interruptActions() {
  eng->renderer->drawMapAndInterface();

  eng->inventoryHandler->screenToOpenAfterDrop = endOfInventoryScreens;
  eng->inventoryHandler->browserPosToSetAfterDrop = 0;

  //Abort searching
  if(waitTurnsLeft > 0) {
    eng->log->addMsg("I stop waiting.", clrWhite);
    eng->renderer->drawMapAndInterface();
  }
  waitTurnsLeft = -1;

  if(activeMedicalBag != NULL) {
    activeMedicalBag->interrupted();
  }
}

void Player::explosiveThrown() {
  dynamiteFuseTurns = -1;
  molotovFuseTurns = -1;
  flareFuseTurns = -1;
  updateColor();
  eng->renderer->drawMapAndInterface();
}

void Player::hearSound(const Sound& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                       const Direction_t directionToOrigin,
                       const int PERCENT_AUDIBLE_DISTANCE) {
  const Sfx_t sfx = snd.getSfx();

  const string& msg = snd.getMsg();
  const bool IS_MSG_EMPTY = msg.empty();

  if(IS_MSG_EMPTY == false) {
    eng->log->addMsg(msg);
  }

  //Play audio after message to ensure synch between audio and animation
  //If origin is hidden, we only play the sound if there is a message
  if(IS_MSG_EMPTY == false || IS_ORIGIN_SEEN_BY_PLAYER) {
    eng->audio->playFromDirection(
      sfx, directionToOrigin, PERCENT_AUDIBLE_DISTANCE);
  }
}

void Player::moveDirection(const Pos& dir) {
  if(deadState == actorDeadState_alive) {

    Pos dest = pos + dir;
    propHandler_->changeMovePos(pos, dest);

    //Trap affects leaving?
    if(dest != pos) {
      Feature* f = eng->map->featuresStatic[pos.x][pos.y];
      if(f->getId() == feature_trap) {
        trace << "Player: Standing on trap, check if affects move" << endl;
        dest = dynamic_cast<Trap*>(f)->actorTryLeave(*this, pos, dest);
      }
    }

    bool isFreeTurn = false;;

    if(dest != pos) {
      //Attack?
      Actor* const actorAtDest = eng->mapTests->getActorAtPos(dest);
      if(actorAtDest != NULL) {
        if(propHandler_->allowAttackMelee(true)) {
          bool hasMeleeWeapon = false;
          Item* const item = inventory_->getItemInSlot(slot_wielded);
          if(item != NULL) {
            Weapon* const weapon = dynamic_cast<Weapon*>(item);
            if(weapon->getData().isMeleeWeapon) {
              if(eng->config->useRangedWpnMeleeePrompt &&
                  checkIfSeeActor(*actorAtDest, NULL)) {
                if(weapon->getData().isRangedWeapon) {
                  const string wpnName =
                    eng->itemDataHandler->getItemRef(*weapon, itemRef_a);
                  eng->log->addMsg(
                    "Attack " + actorAtDest->getNameThe() +
                    " with " + wpnName + "? (y/n)", clrWhiteHigh);
                  eng->renderer->drawMapAndInterface();
                  if(eng->query->yesOrNo() == false) {
                    eng->log->clearLog();
                    eng->renderer->drawMapAndInterface();
                    return;
                  }
                }
              }
              hasMeleeWeapon = true;
              eng->attack->melee(*this, *weapon, *actorAtDest);
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
      bool featuresAllowMove =
        eng->map->featuresStatic[dest.x][dest.y]->isMovePassable(this);
      vector<FeatureMob*> featureMobs =
        eng->gameTime->getFeatureMobsAtPos(dest);
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
          eng->log->addMsg("I am too encumbered to move!");
          eng->renderer->drawMapAndInterface();
          return;
        }

        pos = dest;

        PlayerBonHandler* const bon = eng->playerBonHandler;
        if(bon->isBonPicked(playerBon_dexterous)) {
          const int FREE_MOVE_EVERY_N_TURN = 4;
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
          string message = propHandler_->allowSee() == false ?
                           "I feel here: " : "I see here: ";
          message += eng->itemDataHandler->getItemInterfaceRef(*item, true);
          eng->log->addMsg(message + ".");
        }
      }

      //Note: bump() prints block messages.
      for(unsigned int i = 0; i < featureMobs.size(); i++) {
        featureMobs.at(i)->bump(*this);
      }
      eng->map->featuresStatic[dest.x][dest.y]->bump(*this);
    }
    //If destination reached, then we either moved or were held by something.
    //End turn (unless free turn due to bonus).
    if(pos == dest && isFreeTurn == false) {
      eng->gameTime->endTurnOfCurrentActor();
      return;
    }
    eng->gameTime->updateLightMap();
    updateFov();
    eng->renderer->drawMapAndInterface();
  }
}

void Player::autoMelee() {
  if(target != NULL) {
    if(eng->mapTests->isCellsAdj(pos, target->pos, false)) {
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
        const Actor* const actor = eng->mapTests->getActorAtPos(pos + Pos(dx, dy));
        if(actor != NULL) {
          if(checkIfSeeActor(*actor, NULL)) {
            target = actor;
            moveDirection(Pos(dx, dy));
            return;
          }
        }
      }
    }
  }
}

void Player::kick(Actor& actorToKick) {
  Weapon* kickWeapon = NULL;

  const ActorData* const d = actorToKick.getData();

  if(d->actorSize == actorSize_floor && (d->isSpider || d->isRat)) {
    kickWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerStomp));
  } else {
    kickWeapon = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerKick));
  }
  eng->attack->melee(*this, *kickWeapon, actorToKick);
  delete kickWeapon;
}

void Player::punch(Actor& actorToPunch) {
  //Spawn a temporary punch weapon to attack with
  Weapon* punchWeapon = dynamic_cast<Weapon*>(
                          eng->itemFactory->spawnItem(item_playerPunch));
  eng->attack->melee(*this, *punchWeapon, actorToPunch);
  delete punchWeapon;
}

void Player::actorSpecific_addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {

  bool isUsingLightGivingItem = flareFuseTurns > 0;

  if(isUsingLightGivingItem == false) {
    for(unsigned int i = 0; i < inventory_->getGeneral()->size(); i++) {
      Item* const item = inventory_->getGeneral()->at(i);
      if(item->getData().id == item_deviceElectricLantern) {
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
    Pos x0y0(max(0, pos.x - RADI), max(0, pos.y - RADI));
    Pos x1y1(min(MAP_X_CELLS - 1, pos.x + RADI), min(MAP_Y_CELLS - 1, pos.y + RADI));

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

  if(propHandler_->allowSee()) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeVisionBlockerArray(pos, blockers);
    eng->fov->runPlayerFov(blockers, pos);
    eng->map->playerVision[pos.x][pos.y] = true;
  }

  if(propHandler_->hasProp(propClairvoyant)) {
    const int FLOODFILL_TRAVEL_LIMIT = FOV_STANDARD_RADI_INT + 2;

    const int X0 = max(0, pos.x - FLOODFILL_TRAVEL_LIMIT);
    const int Y0 = max(0, pos.y - FLOODFILL_TRAVEL_LIMIT);
    const int X1 = min(MAP_X_CELLS - 1, pos.x + FLOODFILL_TRAVEL_LIMIT);
    const int Y1 = min(MAP_Y_CELLS - 1, pos.y + FLOODFILL_TRAVEL_LIMIT);

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
      actorBodyType_flying, blockers);

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
          blockers[x][y] = false;
        }
      }
    }

    int floodFillValues[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->floodFill(pos, blockers, floodFillValues, FLOODFILL_TRAVEL_LIMIT, Pos(-1, -1));

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(floodFillValues[x][y]) {
          eng->map->playerVision[x][y] = true;
        }
      }
    }
  }

  if(propHandler_->allowSee()) {FOVhack();}

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
      if(eng->map->playerVision[x][y]) {
        eng->map->explored[x][y] = true;
      }
    }
  }
}

void Player::FOVhack() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(pos, visionBlockers, 9999);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayFeaturesOnly(eng->player, blockers);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(visionBlockers[x][y] && blockers[x][y]) {
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            const Pos adj(x + dx, y + dy);
            if(eng->mapTests->isCellInsideMap(adj)) {
              if(
                eng->map->playerVision[adj.x][adj.y] &&
                (eng->map->darkness[adj.x][adj.y] == false ||
                 eng->map->light[adj.x][adj.y]) &&
                blockers[adj.x][adj.y] == false) {
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
  if(eng->playerSpellsHandler->isSpellLearned(spell_mthPower) == false) {
    eng->playerSpellsHandler->learnSpellIfNotKnown(spell_mthPower);
    string str = "I have gained a deeper insight into the esoteric forces";
    str += " acting behind our apparent reality. With this knowledge, I can";
    str += " attempt to acquire hidden information or displace existence";
    str += " according to my will. Gained spell: Thaumaturgic Alteration";
    eng->popup->showMessage(str, true, "Thaumaturgic Alteration");
  }
}



