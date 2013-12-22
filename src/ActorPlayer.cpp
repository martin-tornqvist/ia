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
#include "Bot.h"
#include "Input.h"
#include "MapParsing.h"

Player::Player(Engine& engine) :
  Actor(engine), activeMedicalBag(NULL), waitTurnsLeft(-1), dynamiteFuseTurns(-1),
  molotovFuseTurns(-1), flareFuseTurns(-1),
  target(NULL), insanity_(0), shock_(0.0), shockTemp_(0.0),
  mth(0), nrMovesUntilFreeAction(-1), carryWeightBase(450) {}

void Player::specificSpawnStartItems() {
  data_->abilityVals.reset();

  for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
    insanityPhobias[i] = false;
  }
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    insanityObsessions[i] = false;
  }

  int NR_CARTRIDGES        = eng.dice.range(1, 2);
  int NR_DYNAMITE          = eng.dice.range(2, 3);
  int NR_MOLOTOV           = eng.dice.range(2, 3);
  int NR_THROWING_KNIVES   = eng.dice.range(7, 12);

  const int WEAPON_CHOICE = eng.dice.range(1, 5);
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
    slot_wielded, eng.itemFactory->spawnItem(weaponId), true, true);

  inventory_->putItemInSlot(
    slot_wieldedAlt, eng.itemFactory->spawnItem(item_pistol), true, true);

  for(int i = 0; i < NR_CARTRIDGES; i++) {
    inventory_->putItemInGeneral(eng.itemFactory->spawnItem(item_pistolClip));
  }

  //TODO Remove:
  //--------------------------------------------------------------------------
//  inventory_->putItemInGeneral(
//    eng.itemFactory->spawnItem(item_machineGun));
//  for(int i = 0; i < 2; i++) {
//    inventory_->putItemInGeneral(
//      eng.itemFactory->spawnItem(item_drumOfBullets));
//  }
//  inventory_->putItemInGeneral(
//    eng.itemFactory->spawnItem(item_sawedOff));
//  inventory_->putItemInGeneral(
//    eng.itemFactory->spawnItem(item_pumpShotgun));
//  inventory_->putItemInGeneral(
//    eng.itemFactory->spawnItem(item_shotgunShell, 80));
  //--------------------------------------------------------------------------

  inventory_->putItemInGeneral(
    eng.itemFactory->spawnItem(item_dynamite, NR_DYNAMITE));
  inventory_->putItemInGeneral(
    eng.itemFactory->spawnItem(item_molotov, NR_MOLOTOV));

  if(NR_THROWING_KNIVES > 0) {
    inventory_->putItemInSlot(
      slot_missiles,
      eng.itemFactory->spawnItem(item_throwingKnife, NR_THROWING_KNIVES),
      true, true);
  }

  inventory_->putItemInSlot(
    slot_armorBody,
    eng.itemFactory->spawnItem(item_armorLeatherJacket),
    true, true);

  inventory_->putItemInGeneral(
    eng.itemFactory->spawnItem(item_deviceElectricLantern));
  inventory_->putItemInGeneral(
    eng.itemFactory->spawnItem(item_medicalBag));
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

void Player::specificHit(const int DMG, const bool ALLOW_WOUNDS) {
  interruptActions();

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

  if(ALLOW_WOUNDS && eng.config->isBotPlaying == false) {
    if(DMG >= 5) {
      Prop* const prop = new PropWound(eng, propTurnsIndefinite);
      propHandler_->tryApplyProp(prop);
    }
  }

  eng.renderer->drawMapAndInterface();
}

int Player::getCarryWeightLimit() const {
  PlayerBonHandler* const bon = eng.playerBonHandler;
  const bool IS_TOUGH         = bon->hasTrait(traitTough);
  const bool IS_STRONG_BACKED = bon->hasTrait(traitStrongBacked);
  const bool IS_WEAKENED      = propHandler_->hasProp(propWeakened);
  const int CARRY_WEIGHT_MOD =
    IS_TOUGH * 10 + IS_STRONG_BACKED * 30 - IS_WEAKENED * 15;

  return (carryWeightBase * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::getShockResistance() const {
  int ret = 0;
  if(eng.playerBonHandler->hasTrait(traitFearless)) {
    ret += 5;
  }
  if(eng.playerBonHandler->hasTrait(traitCoolHeaded)) {
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
  trace << "Player: Increasing insanity" << endl;
  string msg = getInsanity() < 100 ? "Insanity draws nearer... " : "";

  const int INS_INCR = 6;

  if(eng.config->isBotPlaying == false) {
    insanity_ += INS_INCR;
  }

  mth = min(100, mth + INS_INCR / 2);

  restoreShock(70, false);

  updateColor();
  eng.renderer->drawMapAndInterface();

  if(getInsanity() >= 100) {
    msg += "My mind can no longer withstand what it has grasped.";
    msg += " I am hopelessly lost.";
    eng.popup->showMessage(msg, true, "Complete insanity!",
                           sfxInsanityRising);
    die(true, false, false);
  } else {
    bool playerSeeShockingMonster = false;
    vector<Actor*> spotedEnemies;
    getSpotedEnemies(spotedEnemies);
    for(Actor* actor : spotedEnemies) {
      const ActorData& def = actor->getData();
      if(def.monsterShockLevel != monsterShockLevel_none) {
        playerSeeShockingMonster = true;
      }
    }

    //When long term sanity decreases something happens (mostly bad)
    //(Reroll until something actually happens)
    while(true) {
      const int ROLL = eng.dice.range(1, 8);
      switch(ROLL) {
        case 1: {
          if(playerSeeShockingMonster) {
            if(eng.dice.coinToss()) {
              msg += "I let out a terrified shriek.";
            } else {
              msg += "I scream in terror.";
            }
            eng.popup->showMessage(msg, true, "Screaming!",
                                   sfxInsanityRising);
            eng.soundEmitter->emitSound(
              Sound("", endOfSfx, true, pos, true, true));
            return;
          }
        } break;

        case 2: {
          msg += "I find myself babbling incoherently.";
          eng.popup->showMessage(msg, true, "Babbling!",
                                 sfxInsanityRising);
          const string playerName = getNameThe();
          for(int i = eng.dice.range(3, 5); i > 0; i--) {
            const string phrase = Cultist::getCultistPhrase(eng);
            eng.log->addMsg(playerName + ": " + phrase);
          }
          eng.soundEmitter->emitSound(
            Sound("", endOfSfx, true, pos, false, true));
          return;
        } break;

        case 3: {
          msg += "I struggle to not fall into a stupor.";
          eng.popup->showMessage(msg, true, "Fainting!",
                                 sfxInsanityRising);
          propHandler_->tryApplyProp(
            new PropFainted(eng, propTurnsStandard));
          return;
        } break;

        case 4: {
          msg += "I laugh nervously.";
          eng.popup->showMessage(msg, true, "HAHAHA!",
                                 sfxInsanityRising);
          eng.soundEmitter->emitSound(
            Sound("", endOfSfx, true, pos, false, true));
          return;
        } break;

        case 5: {
          if(getPropHandler().hasProp(propRFear) == false) {

            if(insanity_ > 5) {
              //There is a limit to the number of phobias you can have
              int phobiasActive = 0;
              for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
                if(insanityPhobias[i]) {
                  phobiasActive++;
                }
              }
              if(phobiasActive < 2) {
                if(eng.dice.coinToss()) {
                  if(spotedEnemies.empty() == false) {
                    const int MONSTER_ROLL =
                      eng.dice.range(0, spotedEnemies.size() - 1);
                    const ActorData& monsterData =
                      spotedEnemies.at(MONSTER_ROLL)->getData();
                    if(
                      monsterData.isRat &&
                      insanityPhobias[insanityPhobia_rat] == false) {
                      msg += "I am afflicted by Murophobia. ";
                      msg += "Rats suddenly seem terrifying.";
                      eng.popup->showMessage(
                        msg, true, "Murophobia!", sfxInsanityRising);
                      insanityPhobias[insanityPhobia_rat] = true;
                      return;
                    }
                    if(
                      monsterData.isSpider &&
                      insanityPhobias[insanityPhobia_spider] == false) {
                      msg += "I am afflicted by Arachnophobia. ";
                      msg += "Spiders suddenly seem terrifying.";
                      eng.popup->showMessage(
                        msg, true, "Arachnophobia!",
                        sfxInsanityRising);
                      insanityPhobias[insanityPhobia_spider] = true;
                      return;
                    }
                    if(
                      monsterData.isCanine &&
                      insanityPhobias[insanityPhobia_dog] == false) {
                      msg += "I am afflicted by Cynophobia. ";
                      msg += "Dogs suddenly seem terrifying.";
                      eng.popup->showMessage(
                        msg, true, "Cynophobia!", sfxInsanityRising);
                      insanityPhobias[insanityPhobia_dog] = true;
                      return;
                    }
                    if(
                      monsterData.isUndead &&
                      insanityPhobias[insanityPhobia_undead] == false) {
                      msg += "I am afflicted by Necrophobia. ";
                      msg += "The undead suddenly seem much more terrifying.";
                      eng.popup->showMessage(msg, true, "Necrophobia!");
                      insanityPhobias[insanityPhobia_undead] = true;
                      return;
                    }
                  }
                } else {
                  if(eng.dice.coinToss()) {
                    if(isStandingInOpenSpace()) {
                      if(insanityPhobias[insanityPhobia_openPlace] == false) {
                        msg += "I am afflicted by Agoraphobia. ";
                        msg += "Open places suddenly seem terrifying.";
                        eng.popup->showMessage(
                          msg, true, "Agoraphobia!", sfxInsanityRising);
                        insanityPhobias[insanityPhobia_openPlace] = true;
                        return;
                      }
                    }
                    if(isStandingInCrampedSpace()) {
                      if(insanityPhobias[insanityPhobia_closedPlace] == false) {
                        msg += "I am afflicted by Claustrophobia. ";
                        msg += "Confined places suddenly seem terrifying.";
                        eng.popup->showMessage(
                          msg, true, "Claustrophobia!", sfxInsanityRising);
                        insanityPhobias[insanityPhobia_closedPlace] = true;
                        return;
                      }
                    }
                  } else {
                    if(eng.map->getDlvl() >= 5) {
                      if(insanityPhobias[insanityPhobia_deepPlaces] == false) {
                        msg += "I am afflicted by Bathophobia. ";
                        msg += "It suddenly seems terrifying to delve deeper.";
                        eng.popup->showMessage(msg, true, "Bathophobia!");
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
                (InsanityObsession_t)(eng.dice.range(
                                        0, endOfInsanityObsessions - 1));
              switch(obsession) {
                case insanityObsession_masochism: {
                  msg += "To my alarm, I find myself encouraged by the ";
                  msg += "sensation of pain. Every time I am hurt, I find a ";
                  msg += "little relief. However, my depraved mind can no ";
                  msg += "longer find complete peace (shock cannot go below ";
                  msg += toString(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  eng.popup->showMessage(
                    msg, true, "Masochistic obsession!", sfxInsanityRising);
                  insanityObsessions[insanityObsession_masochism] = true;
                  return;
                } break;
                case insanityObsession_sadism: {
                  msg += "To my alarm, I find myself encouraged by the pain ";
                  msg += "I cause in others. For every life I take, I find a ";
                  msg += "little relief. However, my depraved mind can no ";
                  msg += "longer find complete peace (shock cannot go below ";
                  msg += toString(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  eng.popup->showMessage(
                    msg, true, "Sadistic obsession!", sfxInsanityRising);
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
            msg += "The shadows are closing in on me!";
            eng.popup->showMessage(
              msg, true, "Haunted by shadows!", sfxInsanityRising);

            const int NR_SHADOWS_LOWER = 1;
            const int NR_SHADOWS_UPPER =
              getConstrInRange(2, (eng.map->getDlvl() + 1) / 2, 6);
            const int NR_SHADOWS =
              eng.dice.range(NR_SHADOWS_LOWER, NR_SHADOWS_UPPER);

            eng.actorFactory->summonMonsters(
              pos, vector<ActorId_t>(NR_SHADOWS, actor_shadow), true);

            return;
          }
        } break;

        case 8: {
          msg += "I find myself in a peculiar detached daze, ";
          msg += "a tranced state of mind. I struggle to recall ";
          msg += "where I am, or what I'm doing.";
          eng.popup->showMessage(msg, true, "Confusion!", sfxInsanityRising);

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
    eng.map->cells[pos.x][pos.y].isDark &&
    eng.map->cells[pos.x][pos.y].isLight == false) {
    shockTemp_ += 20;
  }

  for(int dy = -1; dy <= 1; dy++) {
    const int Y = pos.y + dy;
    for(int dx = -1; dx <= 1; dx++) {
      const int X = pos.x + dx;
      if(eng.basicUtils->isPosInsideMap(Pos(X, Y))) {
        const Feature* const f =
          eng.map->cells[pos.x + dx][pos.y + dy].featureStatic;
        shockTemp_ += f->getShockWhenAdjacent();
      }
    }
  }
  shockTemp_ = min(99.0, shockTemp_);
}

bool Player::isStandingInOpenSpace() const {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, false, eng),
                   blockers);
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
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, false, eng),
                   blockers);
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

  const int ROLL = eng.dice.percentile();
  //Phobia vs creature type?
  if(ROLL < 10) {
    for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
      const ActorData& monsterData = spotedEnemies.at(0)->getData();
      if(
        monsterData.isCanine &&
        insanityPhobias[insanityPhobia_dog]) {
        eng.log->addMsg("I am plagued by my canine phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng.dice(1, 6)));
        return;
      }
      if(
        monsterData.isRat &&
        insanityPhobias[insanityPhobia_rat]) {
        eng.log->addMsg("I am plagued by my rat phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng.dice(1, 6)));
        return;
      }
      if(
        monsterData.isUndead &&
        insanityPhobias[insanityPhobia_undead]) {
        eng.log->addMsg("I am plagued by my phobia of the dead!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng.dice(1, 6)));
        return;
      }
      if(
        monsterData.isSpider &&
        insanityPhobias[insanityPhobia_spider]) {
        eng.log->addMsg("I am plagued by my spider phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng.dice(1, 6)));
        return;
      }
    }
  }
  if(ROLL < 5) {
    if(insanityPhobias[insanityPhobia_openPlace]) {
      if(isStandingInOpenSpace()) {
        eng.log->addMsg("I am plagued by my phobia of open places!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng.dice(1, 6)));
        return;
      }
    }

    if(insanityPhobias[insanityPhobia_closedPlace]) {
      if(isStandingInCrampedSpace()) {
        eng.log->addMsg("I am plagued by my phobia of closed places!");
        propHandler_->tryApplyProp(
          new PropTerrified(eng, propTurnsSpecified, eng.dice(1, 6)));
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

void Player::onActorTurn() {
  shockTemp_ = 0;
  setTempShockFromFeatures();

  eng.renderer->drawMapAndInterface();

  if(deadState != actorDeadState_alive) {
    return;
  }

  if(getMth() >= 15) {
    grantMthPower();
  }

  //If player dropped item, check if should go back to inventory screen
  vector<Actor*> spotedEnemies;
  getSpotedEnemies(spotedEnemies);
  if(spotedEnemies.empty()) {
    const InventoryScreen_t invScreen =
      eng.inventoryHandler->screenToOpenAfterDrop;
    if(invScreen != endOfInventoryScreens) {
      switch(invScreen) {
        case inventoryScreen_backpack: {
          eng.inventoryHandler->runBrowseInventoryMode();
        } break;

        case inventoryScreen_use: {
          eng.inventoryHandler->runUseScreen();
        } break;

        case inventoryScreen_equip: {
          eng.inventoryHandler->runEquipScreen(
            eng.inventoryHandler->equipSlotToOpenAfterDrop);
        } break;

        case inventoryScreen_slots: {
          eng.inventoryHandler->runSlotsScreen();
        } break;

        case endOfInventoryScreens: {} break;
      }
      return;
    }
  } else {
    eng.inventoryHandler->screenToOpenAfterDrop = endOfInventoryScreens;
    eng.inventoryHandler->browserPosToSetAfterDrop = 0;
  }

  if(eng.config->isBotPlaying) {
    eng.bot->act();
  } else {
    eng.input->clearEvents();
    eng.input->handleMapModeInputUntilFound();
  }
}

void Player::specificOnStandardTurn() {
  // Dynamite
  if(dynamiteFuseTurns > 0) {
    dynamiteFuseTurns--;
    if(dynamiteFuseTurns > 0) {
      string fuseMessage = "***F";
      for(int i = 0; i < dynamiteFuseTurns; i++) {
        fuseMessage += "Z";
      }
      fuseMessage += "***";
      eng.log->addMsg(fuseMessage, clrYellow);
    }
  }
  if(dynamiteFuseTurns == 0) {
    eng.log->addMsg("The dynamite explodes in my hands!");
    eng.explosionMaker->runExplosion(pos);
    updateColor();
    dynamiteFuseTurns = -1;
  }

  //Molotovs
  if(molotovFuseTurns > 0) {
    molotovFuseTurns--;
  }
  if(molotovFuseTurns == 0) {
    eng.log->addMsg("The Molotov Cocktail explodes in my hands!");
    molotovFuseTurns = -1;
    updateColor();
    eng.explosionMaker->runExplosion(
      pos, sfxExplosionMolotov, false,
      new PropBurning(eng, propTurnsStandard));
  }

  //Flare
  if(flareFuseTurns > 0) {
    flareFuseTurns--;
  }
  if(flareFuseTurns == 0) {
    eng.log->addMsg("The flare is extinguished.");
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
  for(Actor* actor : spotedEnemies) {
    Monster* monster = dynamic_cast<Monster*>(actor);
    const ActorData& data = monster->getData();
    if(data.monsterShockLevel != monsterShockLevel_none) {
      switch(data.monsterShockLevel) {
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
  const int TURN = eng.gameTime->getTurn();
  const int LOSE_N_TURN = 14;
  if((TURN / LOSE_N_TURN) * LOSE_N_TURN == TURN && TURN > 1) {
    if(eng.dice(1, 1000) <= 2) {
      if(eng.dice.coinToss()) {
        eng.popup->showMessage("I have a bad feeling about this...", true);
      } else {
        eng.popup->showMessage("A chill runs down my spine...", true);
      }
      incrShock(shockValue_heavy);
      eng.renderer->drawMapAndInterface();
    } else {
      if(eng.map->getDlvl() != 0) {
        incrShock(1);
      }
    }
  }

  //Take sanity hit from high shock?
  if(getShockTotal() >= 100) {
    incrInsanity();
    eng.gameTime->endTurnOfCurrentActor();
    return;
  }

  const int NR_ACTORS = eng.gameTime->getNrActors();

  for(int i = 0; i < NR_ACTORS; i++) {
    //If applying first aid etc, messages may be printed for monsters that
    //comes into view.
    //Only print monster-comes-into-view-messages if player is busy with
    //something (first aid, auto travel etc).

    Actor* const actor = &(eng.gameTime->getActorAtElement(i));
    if(actor != this) {
      if(actor->deadState == actorDeadState_alive) {

        Monster* const monster = dynamic_cast<Monster*>(actor);
        const bool IS_MONSTER_SEEN = checkIfSeeActor(*actor, NULL);
        if(IS_MONSTER_SEEN) {
          if(monster->messageMonsterInViewPrinted == false) {
            if(activeMedicalBag != NULL || waitTurnsLeft > 0) {
              eng.log->addMsg(actor->getNameA() + " comes into my view.",
                              clrWhite, true);
            }
            monster->messageMonsterInViewPrinted = true;
          }
        } else {
          monster->messageMonsterInViewPrinted = false;

          //Is the monster sneaking? Try to spot it
          if(eng.map->cells[monster->pos.x][monster->pos.y].isSeenByPlayer) {
            if(monster->isStealth) {
              const int PLAYER_SEARCH_SKILL =
                data_->abilityVals.getVal(ability_searching, true, *this);
              const AbilityRollResult_t rollResult =
                eng.abilityRoll->roll(PLAYER_SEARCH_SKILL);
              if(rollResult == successSmall) {
                eng.log->addMsg("I see something moving in the shadows.");
              } else if(rollResult > successSmall) {
                monster->isStealth = false;
                updateFov();
                eng.renderer->drawMapAndInterface();
                eng.log->addMsg("I spot " + monster->getNameA() + "!");
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
        eng.playerBonHandler->hasTrait(traitRapidRecoverer);

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
          if(eng.map->cells[x][y].isSeenByPlayer) {
            Feature* f = eng.map->cells[x][y].featureStatic;

            if(f->getId() == feature_trap) {
              dynamic_cast<Trap*>(f)->playerTrySpotHidden();
            }
            if(f->getId() == feature_door) {
              dynamic_cast<Door*>(f)->playerTrySpotHidden();
            }
          }
        }
      }

      if(eng.playerBonHandler->hasTrait(traitObservant)) {
        const int CLUE_RADI = 3;
        x0 = max(0, pos.x - CLUE_RADI);
        y0 = max(0, pos.y - CLUE_RADI);
        x1 = min(MAP_X_CELLS - 1, pos.x + CLUE_RADI);
        y1 = max(MAP_Y_CELLS - 1, pos.y + CLUE_RADI);

        for(int y = y0; y <= y1; y++) {
          for(int x = x0; x <= x1; x++) {
            if(eng.map->cells[x][y].isSeenByPlayer) {
              Feature* f = eng.map->cells[x][y].featureStatic;
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
    eng.renderer->drawMapAndInterface();
    if(DELAY_PLAYER_WAITING > 0) {
      eng.sleep(DELAY_PLAYER_WAITING);
    }
    activeMedicalBag->continueAction();
  }

  if(waitTurnsLeft > 0) {
    eng.renderer->drawMapAndInterface();
    if(DELAY_PLAYER_WAITING > 0) {
      eng.sleep(DELAY_PLAYER_WAITING);
    }
    waitTurnsLeft--;
    eng.gameTime->endTurnOfCurrentActor();
  }
}

void Player::interruptActions() {
  eng.renderer->drawMapAndInterface();

  eng.inventoryHandler->screenToOpenAfterDrop = endOfInventoryScreens;
  eng.inventoryHandler->browserPosToSetAfterDrop = 0;

  //Abort searching
  if(waitTurnsLeft > 0) {
    eng.log->addMsg("I stop waiting.", clrWhite);
    eng.renderer->drawMapAndInterface();
  }
  waitTurnsLeft = -1;

  if(activeMedicalBag != NULL) {
    activeMedicalBag->interrupted();
    activeMedicalBag = NULL;
  }
}

void Player::explosiveThrown() {
  dynamiteFuseTurns = -1;
  molotovFuseTurns = -1;
  flareFuseTurns = -1;
  updateColor();
  eng.renderer->drawMapAndInterface();
}

void Player::hearSound(const Sound& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                       const Dir_t dirToOrigin,
                       const int PERCENT_AUDIBLE_DISTANCE) {
  const Sfx_t sfx = snd.getSfx();

  const string& msg = snd.getMsg();
  const bool IS_MSG_EMPTY = msg.empty();

  if(IS_MSG_EMPTY == false) {
    eng.log->addMsg(msg, clrWhite, true);
  }

  //Play audio after message to ensure synch between audio and animation
  //If origin is hidden, we only play the sound if there is a message
  if(IS_MSG_EMPTY == false || IS_ORIGIN_SEEN_BY_PLAYER) {
    eng.audio->playFromDir(
      sfx, dirToOrigin, PERCENT_AUDIBLE_DISTANCE);
  }
}

void Player::moveDir(Dir_t dir) {
  if(deadState == actorDeadState_alive) {

    propHandler_->changeMoveDir(pos, dir);

    //Trap affects leaving?
    if(dir != dirCenter) {
      Feature* f = eng.map->cells[pos.x][pos.y].featureStatic;
      if(f->getId() == feature_trap) {
        trace << "Player: Standing on trap, check if affects move" << endl;
        dir = dynamic_cast<Trap*>(f)->actorTryLeave(*this, dir);
      }
    }

    bool isFreeTurn = false;;

    const Pos dest(pos + DirConverter().getOffset(dir));

    if(dir != dirCenter) {
      //Attack?
      Actor* const actorAtDest = eng.basicUtils->getActorAtPos(dest);
      if(actorAtDest != NULL) {
        if(propHandler_->allowAttackMelee(true)) {
          bool hasMeleeWeapon = false;
          Item* const item = inventory_->getItemInSlot(slot_wielded);
          if(item != NULL) {
            Weapon* const weapon = dynamic_cast<Weapon*>(item);
            if(weapon->getData().isMeleeWeapon) {
              if(eng.config->useRangedWpnMeleeePrompt &&
                  checkIfSeeActor(*actorAtDest, NULL)) {
                if(weapon->getData().isRangedWeapon) {
                  const string wpnName =
                    eng.itemDataHandler->getItemRef(*weapon, itemRef_a);
                  eng.log->addMsg(
                    "Attack " + actorAtDest->getNameThe() +
                    " with " + wpnName + "? (y/n)", clrWhiteHigh);
                  eng.renderer->drawMapAndInterface();
                  if(eng.query->yesOrNo() == false) {
                    eng.log->clearLog();
                    eng.renderer->drawMapAndInterface();
                    return;
                  }
                }
              }
              hasMeleeWeapon = true;
              eng.attack->melee(*this, *weapon, *actorAtDest);
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
        eng.map->cells[dest.x][dest.y].featureStatic->isMovePassable(this);
      vector<FeatureMob*> featureMobs =
        eng.gameTime->getFeatureMobsAtPos(dest);
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
          eng.log->addMsg("I am too encumbered to move!");
          eng.renderer->drawMapAndInterface();
          return;
        }

        pos = dest;

        PlayerBonHandler* const bon = eng.playerBonHandler;
        if(bon->hasTrait(traitDexterous)) {
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
        Item* const item = eng.map->cells[pos.x][pos.y].item;
        if(item != NULL) {
          string message = propHandler_->allowSee() == false ?
                           "I feel here: " : "I see here: ";
          message += eng.itemDataHandler->getItemInterfaceRef(*item, true);
          eng.log->addMsg(message + ".");
        }
      }

      //Note: bump() prints block messages.
      for(unsigned int i = 0; i < featureMobs.size(); i++) {
        featureMobs.at(i)->bump(*this);
      }
      eng.map->cells[dest.x][dest.y].featureStatic->bump(*this);
    }
    //If destination reached, then we either moved or were held by something.
    //End turn (unless free turn due to bonus).
    if(pos == dest && isFreeTurn == false) {
      eng.gameTime->endTurnOfCurrentActor();
      return;
    }
    eng.gameTime->updateLightMap();
    updateFov();
    eng.renderer->drawMapAndInterface();
  }
}

void Player::autoMelee() {
  if(target != NULL) {
    if(eng.basicUtils->isPosAdj(pos, target->pos, false)) {
      if(checkIfSeeActor(*target, NULL)) {
        moveDir(
          DirConverter().getDir(target->pos - pos));
        return;
      }
    }
  }

  //If this line reached, there is no adjacent current target.
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      if(dx != 0 || dy != 0) {
        const Actor* const actor =
          eng.basicUtils->getActorAtPos(pos + Pos(dx, dy));
        if(actor != NULL) {
          if(checkIfSeeActor(*actor, NULL)) {
            target = actor;
            moveDir(
              DirConverter().getDir(Pos(dx, dy)));
            return;
          }
        }
      }
    }
  }
}

void Player::kick(Actor& actorToKick) {
  Weapon* kickWeapon = NULL;

  const ActorData& d = actorToKick.getData();

  if(d.actorSize == actorSize_floor && (d.isSpider || d.isRat)) {
    kickWeapon =
      dynamic_cast<Weapon*>(eng.itemFactory->spawnItem(item_playerStomp));
  } else {
    kickWeapon =
      dynamic_cast<Weapon*>(eng.itemFactory->spawnItem(item_playerKick));
  }
  eng.attack->melee(*this, *kickWeapon, actorToKick);
  delete kickWeapon;
}

void Player::punch(Actor& actorToPunch) {
  //Spawn a temporary punch weapon to attack with
  Weapon* punchWeapon = dynamic_cast<Weapon*>(
                          eng.itemFactory->spawnItem(item_playerPunch));
  eng.attack->melee(*this, *punchWeapon, actorToPunch);
  delete punchWeapon;
}

void Player::specificAddLight(
  bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {

  bool isUsingLightGivingItem = flareFuseTurns > 0;

  if(isUsingLightGivingItem == false) {
    vector<Item*>& generalItems = inventory_->getGeneral();
    for(Item* const item : generalItems) {
      if(item->getData().id == item_deviceElectricLantern) {
        DeviceElectricLantern* const lantern =
          dynamic_cast<DeviceElectricLantern*>(item);
        if(lantern->isGivingLight()) {
          isUsingLightGivingItem = true;
          break;
        }
      }
    }
  }

  if(isUsingLightGivingItem) {
    bool myLight[MAP_X_CELLS][MAP_Y_CELLS];
    eng.basicUtils->resetArray(myLight, false);
    const int RADI = FOV_STANDARD_RADI_INT; //LitFlare::getLightRadius();
    Pos x0y0(max(0, pos.x - RADI), max(0, pos.y - RADI));
    Pos x1y1(min(MAP_X_CELLS - 1, pos.x + RADI), min(MAP_Y_CELLS - 1, pos.y + RADI));

    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      for(int x = x0y0.x; x <= x1y1.x; x++) {
        const FeatureStatic* const f = eng.map->cells[x][y].featureStatic;
        visionBlockers[x][y] = f->isVisionPassable() == false;
      }
    }

    eng.fov->runFovOnArray(visionBlockers, pos, myLight, false);
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
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng.map->cells[x][y].isSeenByPlayer = false;
    }
  }

  if(propHandler_->allowSee()) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(CellPredBlocksVision(eng), blockers);
    eng.fov->runPlayerFov(blockers, pos);
    eng.map->cells[pos.x][pos.y].isSeenByPlayer = true;
  }

  if(propHandler_->hasProp(propClairvoyant)) {
    const int FLOODFILL_TRAVEL_LIMIT = FOV_STANDARD_RADI_INT + 2;

    const int X0 = max(0, pos.x - FLOODFILL_TRAVEL_LIMIT);
    const int Y0 = max(0, pos.y - FLOODFILL_TRAVEL_LIMIT);
    const int X1 = min(MAP_X_CELLS - 1, pos.x + FLOODFILL_TRAVEL_LIMIT);
    const int Y1 = min(MAP_Y_CELLS - 1, pos.y + FLOODFILL_TRAVEL_LIMIT);

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(CellPredBlocksBodyType(bodyType_flying, false, eng),
                     blockers);

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(eng.map->cells[x][y].featureStatic->getId() == feature_door) {
          blockers[x][y] = false;
        }
      }
    }

    int floodFillValues[MAP_X_CELLS][MAP_Y_CELLS];
    eng.floodFill->run(
      pos, blockers, floodFillValues, FLOODFILL_TRAVEL_LIMIT, Pos(-1, -1));

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(floodFillValues[x][y]) {
          eng.map->cells[x][y].isSeenByPlayer = true;
        }
      }
    }
  }

  if(propHandler_->allowSee()) {FOVhack();}

  if(eng.isCheatVisionEnabled) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        eng.map->cells[x][y].isSeenByPlayer = true;
      }
    }
  }

  //Explore
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      if(eng.map->cells[x][y].isSeenByPlayer) {
        eng.map->cells[x][y].isExplored = true;
      }
    }
  }
}

void Player::FOVhack() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, false, eng),
                   blockers);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(visionBlockers[x][y] && blockers[x][y]) {
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            const Pos adj(x + dx, y + dy);
            if(eng.basicUtils->isPosInsideMap(adj)) {
              const Cell& adjCell = eng.map->cells[adj.x][adj.y];
              if(
                adjCell.isSeenByPlayer &&
                (adjCell.isDark == false || adjCell.isLight) &&
                blockers[adj.x][adj.y] == false) {
                eng.map->cells[x][y].isSeenByPlayer = true;
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
  if(eng.playerSpellsHandler->isSpellLearned(spell_mthPower) == false) {
    eng.playerSpellsHandler->learnSpellIfNotKnown(spell_mthPower);
    string str = "I have gained a deeper insight into the esoteric forces";
    str += " acting behind our apparent reality. With this knowledge, I can";
    str += " attempt to acquire hidden information or displace existence";
    str += " according to my will. Gained spell: Thaumaturgic Alteration";
    eng.popup->showMessage(str, true, "Thaumaturgic Alteration");
  }
}



