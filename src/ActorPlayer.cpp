#include "ActorPlayer.h"

#include "Init.h"
#include "Renderer.h"
#include "Audio.h"
#include "ItemWeapon.h"
#include "FeatureTrap.h"
#include "CreateCharacter.h"
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
#include "Fov.h"
#include "ItemFactory.h"
#include "ActorFactory.h"
#include "PlayerBon.h"
#include "FeatureLitDynamite.h"
#include "Inventory.h"
#include "InventoryHandling.h"
#include "ItemMedicalBag.h"
#include "PlayerSpellsHandler.h"
#include "Bot.h"
#include "Input.h"
#include "MapParsing.h"
#include "Properties.h"
#include "ItemDevice.h"

using namespace std;

const int MIN_SHOCK_WHEN_OBSESSION = 35;

Player::Player() :
  Actor(),
  activeMedicalBag(NULL),
  waitTurnsLeft(-1),
  dynamiteFuseTurns(-1),
  molotovFuseTurns(-1),
  flareFuseTurns(-1),
  target(NULL),
  insanity_(0),
  shock_(0.0),
  shockTmp_(0.0),
  permShockTakenCurTurn_(0.0),
  nrMovesUntilFreeAction_(-1),
  nrTurnsUntilIns_(-1),
  CARRY_WEIGHT_BASE_(450) {}

void Player::spawnStartItems() {
  data_->abilityVals.reset();

  for(int i = 0; i < int(Phobia::endOfPhobias); i++) {
    phobias[i] = false;
  }
  for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
    obsessions[i] = false;
  }

  int NR_CARTRIDGES        = Rnd::range(1, 2);
  int NR_DYNAMITE          = Rnd::range(2, 3);
  int NR_MOLOTOV           = Rnd::range(2, 3);
  int NR_THROWING_KNIVES   = Rnd::range(7, 12);

  const int WEAPON_CHOICE = Rnd::range(1, 5);
  ItemId weaponId = ItemId::dagger;
  switch(WEAPON_CHOICE) {
    case 1:   weaponId = ItemId::dagger;   break;
    case 2:   weaponId = ItemId::hatchet;  break;
    case 3:   weaponId = ItemId::hammer;   break;
    case 4:   weaponId = ItemId::machete;  break;
    case 5:   weaponId = ItemId::axe;      break;
    default:  weaponId = ItemId::dagger;   break;
  }

  inv_->putInSlot(SlotId::wielded, ItemFactory::spawnItem(weaponId), true,
                  true);

  inv_->putInSlot(SlotId::wieldedAlt, ItemFactory::spawnItem(ItemId::pistol),
                  true, true);

  for(int i = 0; i < NR_CARTRIDGES; i++) {
    inv_->putInGeneral(ItemFactory::spawnItem(ItemId::pistolClip));
  }

  //TODO Remove:
  //--------------------------------------------------------------------------
//  inv_->putInGeneral(
//    ItemFactory::spawnItem(ItemId::machineGun));
//  for(int i = 0; i < 2; i++) {
//    inv_->putInGeneral(
//      ItemFactory::spawnItem(ItemId::drumOfBullets));
//  }
//  inv_->putInGeneral(
//    ItemFactory::spawnItem(ItemId::sawedOff));
//  inv_->putInGeneral(
//    ItemFactory::spawnItem(ItemId::pumpShotgun));
//  inv_->putInGeneral(
//    ItemFactory::spawnItem(ItemId::shotgunShell, 80));
  //--------------------------------------------------------------------------

  inv_->putInGeneral(ItemFactory::spawnItem(ItemId::dynamite, NR_DYNAMITE));
  inv_->putInGeneral(ItemFactory::spawnItem(ItemId::molotov, NR_MOLOTOV));

  if(NR_THROWING_KNIVES > 0) {
    inv_->putInSlot(
      SlotId::missiles,
      ItemFactory::spawnItem(ItemId::throwingKnife, NR_THROWING_KNIVES),
      true, true);
  }

  inv_->putInSlot(SlotId::armorBody,
                  ItemFactory::spawnItem(ItemId::armorLeatherJacket),
                  true, true);

  inv_->putInGeneral(ItemFactory::spawnItem(ItemId::electricLantern));
  inv_->putInGeneral(ItemFactory::spawnItem(ItemId::medicalBag));
}

void Player::storeToSaveLines(vector<string>& lines) const {
  lines.push_back(toStr(propHandler_->appliedProps_.size()));
  for(Prop * prop : propHandler_->appliedProps_) {
    lines.push_back(toStr(prop->getId()));
    lines.push_back(toStr(prop->turnsLeft_));
    prop->storeToSaveLines(lines);
  }

  lines.push_back(toStr(insanity_));
  lines.push_back(toStr(int(shock_)));
  lines.push_back(toStr(hp_));
  lines.push_back(toStr(hpMax_));
  lines.push_back(toStr(spi_));
  lines.push_back(toStr(spiMax_));
  lines.push_back(toStr(pos.x));
  lines.push_back(toStr(pos.y));
  lines.push_back(toStr(dynamiteFuseTurns));
  lines.push_back(toStr(molotovFuseTurns));
  lines.push_back(toStr(flareFuseTurns));

  for(int i = 0; i < int(AbilityId::endOfAbilityId); i++) {
    lines.push_back(toStr(data_->abilityVals.getRawVal(AbilityId(i))));
  }

  for(int i = 0; i < int(Phobia::endOfPhobias); i++) {
    lines.push_back(phobias[i] == 0 ? "0" : "1");
  }
  for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
    lines.push_back(obsessions[i] == 0 ? "0" : "1");
  }
}

void Player::setupFromSaveLines(vector<string>& lines) {
  const int NR_PROPS = toInt(lines.front());
  lines.erase(lines.begin());
  for(int i = 0; i < NR_PROPS; i++) {
    const PropId id = PropId(toInt(lines.front()));
    lines.erase(lines.begin());
    const int NR_TURNS = toInt(lines.front());
    lines.erase(lines.begin());
    Prop* const prop = propHandler_->makeProp(
                         id, propTurnsSpecific, NR_TURNS);
    propHandler_->tryApplyProp(prop, true, true, true, true);
    prop->setupFromSaveLines(lines);
  }

  insanity_ = toInt(lines.front());
  lines.erase(lines.begin());
  shock_ = double(toInt(lines.front()));
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

  for(int i = 0; i < int(AbilityId::endOfAbilityId); i++) {
    data_->abilityVals.setVal(AbilityId(i), toInt(lines.front()));
    lines.erase(lines.begin());
  }

  for(int i = 0; i < int(Phobia::endOfPhobias); i++) {
    phobias[i] = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());
  }
  for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
    obsessions[i] = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());
  }
}

void Player::hit_(int& dmg, const bool ALLOW_WOUNDS) {
  if(obsessions[int(Obsession::masochism)] == false) {
    incrShock(1, ShockSrc::misc);
  }

  if(ALLOW_WOUNDS && Config::isBotPlaying() == false) {
    if(dmg >= 5) {
      Prop* const prop = new PropWound(propTurnsIndefinite);
      propHandler_->tryApplyProp(prop);
    }
  }

  Renderer::drawMapAndInterface();
}

int Player::getEncPercent() const {
  const int TOTAL_W = inv_->getTotalItemWeight();
  const int MAX_W   = getCarryWeightLimit();
  return int((double(TOTAL_W) / double(MAX_W)) * 100.0);
}

int Player::getCarryWeightLimit() const {
  const bool IS_TOUGH         = PlayerBon::hasTrait(Trait::tough);
  const bool IS_STRONG_BACKED = PlayerBon::hasTrait(Trait::strongBacked);

  vector<PropId> props;
  propHandler_->getAllActivePropIds(props);
  const bool IS_WEAKENED =
    find(props.begin(), props.end(), propWeakened) != props.end();

  const int CARRY_WEIGHT_MOD =
    (IS_TOUGH * 10) + (IS_STRONG_BACKED * 30) - (IS_WEAKENED * 15);

  return (CARRY_WEIGHT_BASE_ * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::getShockResistance(const ShockSrc shockSrc) const {
  int res = 0;
  if(PlayerBon::hasTrait(Trait::fearless))    {res += 5;}
  if(PlayerBon::hasTrait(Trait::coolHeaded))  {res += 20;}
  if(PlayerBon::hasTrait(Trait::courageous))  {res += 20;}

  switch(shockSrc) {
    case ShockSrc::castIntrSpell: {} break;

    case ShockSrc::useStrangeItem: {
      if(PlayerBon::getBg() == Bg::occultist) {res += 50;}
    } break;

    case ShockSrc::seeMonster:
    case ShockSrc::time:
    case ShockSrc::misc:
    case ShockSrc::endOfShockSrc: {} break;
  }

  return getConstrInRange(0, res, 100);
}

double Player::getShockTakenAfterMods(const int BASE_SHOCK,
                                      const ShockSrc shockSrc) const {
  const double SHOCK_RES_DB   = double(getShockResistance(shockSrc));
  const double BASE_SHOCK_DB  = double(BASE_SHOCK);
  return (BASE_SHOCK_DB * (100.0 - SHOCK_RES_DB)) / 100.0;
}

void Player::incrShock(const int SHOCK, ShockSrc shockSrc) {
  const double SHOCK_AFTER_MODS = getShockTakenAfterMods(SHOCK, shockSrc);

  shock_                  += SHOCK_AFTER_MODS;
  permShockTakenCurTurn_  += SHOCK_AFTER_MODS;

  constrInRange(0.0f, shock_, 100.0f);
}

void Player::incrShock(const ShockValue shockValue, ShockSrc shockSrc) {
  incrShock(int(shockValue), shockSrc);
}

void Player::restoreShock(const int amountRestored,
                          const bool IS_TEMP_SHOCK_RESTORED) {
  // If an obsession is active, only restore to a certain min level
  bool isObsessionActive = 0;
  for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
    if(obsessions[i]) {
      isObsessionActive = true;
      break;
    }
  }
  const double MIN_SHOCK_WHEN_OBSESSION_DB =
    double(MIN_SHOCK_WHEN_OBSESSION);
  shock_ = max(
             (isObsessionActive ? MIN_SHOCK_WHEN_OBSESSION_DB : 0.0),
             shock_ - amountRestored);
  if(IS_TEMP_SHOCK_RESTORED) {shockTmp_ = 0;}
}

void Player::incrInsanity() {
  trace << "Player: Increasing insanity" << endl;
  string msg = getInsanity() < 100 ? "Insanity draws nearer... " : "";

  if(Config::isBotPlaying() == false) {
    const int INS_INCR = 6;
    insanity_ += INS_INCR;
  }

  restoreShock(70, false);

  updateColor();
  Renderer::drawMapAndInterface();

  if(getInsanity() >= 100) {
    msg += "My mind can no longer withstand what it has grasped. "
           "I am hopelessly lost.";
    Popup::showMsg(msg, true, "Complete insanity!", SfxId::insanityRising);
    die(true, false, false);
  } else {
    bool playerSeeShockingMonster = false;
    vector<Actor*> SpottedEnemies;
    getSpottedEnemies(SpottedEnemies);
    for(Actor * actor : SpottedEnemies) {
      const ActorDataT& def = actor->getData();
      if(def.monsterShockLevel != MonsterShockLevel::none) {
        playerSeeShockingMonster = true;
      }
    }

    //When long term sanity decreases something happens (mostly bad)
    //(Reroll until something actually happens)
    while(true) {
      const int ROLL = Rnd::range(1, 8);
      switch(ROLL) {
        case 1: {
          if(playerSeeShockingMonster) {
            if(Rnd::coinToss()) {
              msg += "I let out a terrified shriek.";
            } else {
              msg += "I scream in terror.";
            }
            Popup::showMsg(msg, true, "Screaming!", SfxId::insanityRising);
            Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, pos, this,
                    SndVol::high, AlertsMonsters::yes);
            SndEmit::emitSnd(snd);
            return;
          }
        } break;

        case 2: {
          msg += "I find myself babbling incoherently.";
          Popup::showMsg(msg, true, "Babbling!", SfxId::insanityRising);
          const string playerName = getNameThe();
          for(int i = Rnd::range(3, 5); i > 0; i--) {
            const string phrase = Cultist::getCultistPhrase();
            Log::addMsg(playerName + ": " + phrase);
          }
          Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, pos, this,
                  SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          return;
        } break;

        case 3: {
          msg += "I struggle to not fall into a stupor.";
          Popup::showMsg(msg, true, "Fainting!",
                         SfxId::insanityRising);
          propHandler_->tryApplyProp(new PropFainted(propTurnsStd));
          return;
        } break;

        case 4: {
          msg += "I laugh nervously.";
          Popup::showMsg(msg, true, "HAHAHA!", SfxId::insanityRising);
          Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, pos, this,
                  SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          return;
        } break;

        case 5: {
          vector<PropId> props;
          propHandler_->getAllActivePropIds(props);

          if(find(props.begin(), props.end(), propRFear) != props.end()) {

            if(insanity_ > 5) {
              //There is a limit to the number of phobias you can have
              int phobiasActive = 0;
              for(int i = 0; i < int(Phobia::endOfPhobias); i++) {
                if(phobias[i]) {phobiasActive++;}
              }
              if(phobiasActive < 2) {
                if(Rnd::coinToss()) {
                  if(SpottedEnemies.empty() == false) {
                    const int MONSTER_ROLL =
                      Rnd::range(0, SpottedEnemies.size() - 1);
                    const ActorDataT& monsterData =
                      SpottedEnemies.at(MONSTER_ROLL)->getData();
                    if(
                      monsterData.isRat &&
                      phobias[int(Phobia::rat)] == false) {
                      msg += "I am afflicted by Murophobia. "
                             "Rats suddenly seem terrifying.";
                      Popup::showMsg(msg, true, "Murophobia!",
                                     SfxId::insanityRising);
                      phobias[int(Phobia::rat)] = true;
                      return;
                    }
                    if(
                      monsterData.isSpider &&
                      phobias[int(Phobia::spider)] == false) {
                      msg += "I am afflicted by Arachnophobia. "
                             "Spiders suddenly seem terrifying.";
                      Popup::showMsg(msg, true, "Arachnophobia!",
                                     SfxId::insanityRising);
                      phobias[int(Phobia::spider)] = true;
                      return;
                    }
                    if(
                      monsterData.isCanine &&
                      phobias[int(Phobia::dog)] == false) {
                      msg += "I am afflicted by Cynophobia. "
                             "Dogs suddenly seem terrifying.";
                      Popup::showMsg(msg, true, "Cynophobia!",
                                     SfxId::insanityRising);
                      phobias[int(Phobia::dog)] = true;
                      return;
                    }
                    if(
                      monsterData.isUndead &&
                      phobias[int(Phobia::undead)] == false) {
                      msg += "I am afflicted by Necrophobia. "
                             "The undead suddenly seem much more terrifying.";
                      Popup::showMsg(msg, true, "Necrophobia!");
                      phobias[int(Phobia::undead)] = true;
                      return;
                    }
                  }
                } else {
                  if(Rnd::coinToss()) {
                    if(isStandingInOpenSpace()) {
                      if(phobias[int(Phobia::openPlace)] == false) {
                        msg += "I am afflicted by Agoraphobia. "
                               "Open places suddenly seem terrifying.";
                        Popup::showMsg(msg, true, "Agoraphobia!",
                                       SfxId::insanityRising);
                        phobias[int(Phobia::openPlace)] = true;
                        return;
                      }
                    }
                    if(isStandingInCrampedSpace()) {
                      if(phobias[int(Phobia::closedPlace)] == false) {
                        msg += "I am afflicted by Claustrophobia. "
                               "Confined places suddenly seem terrifying.";
                        Popup::showMsg(msg, true, "Claustrophobia!",
                                       SfxId::insanityRising);
                        phobias[int(Phobia::closedPlace)] = true;
                        return;
                      }
                    }
                  } else {
                    if(Map::dlvl >= 5) {
                      if(phobias[int(Phobia::deepPlaces)] == false) {
                        msg += "I am afflicted by Bathophobia. "
                               "It suddenly seems terrifying to delve deeper.";
                        Popup::showMsg(msg, true, "Bathophobia!");
                        phobias[int(Phobia::deepPlaces)] = true;
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
            for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
              if(obsessions[i]) {obsessionsActive++;}
            }
            if(obsessionsActive == 0) {
              const Obsession obsession =
                (Obsession)(Rnd::range(0, int(Obsession::endOfObsessions) - 1));
              switch(obsession) {
                case Obsession::masochism: {
                  msg +=
                    "To my alarm, I find myself encouraged by the sensation of "
                    "pain. Physical suffering does not frighten me at all. "
                    "However, my depraved mind can never find complete peace "
                    "(no shock from taking damage, but shock cannot go below "
                    + toStr(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  Popup::showMsg(msg, true, "Masochistic obsession!",
                                 SfxId::insanityRising);
                  obsessions[int(Obsession::masochism)] = true;
                  return;
                } break;
                case Obsession::sadism: {
                  msg +=
                    "To my alarm, I find myself encouraged by the pain I cause "
                    "in others. For every life I take, I find a little relief. "
                    "However, my depraved mind can no longer find complete "
                    "peace (shock cannot go below "
                    + toStr(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  Popup::showMsg(msg, true, "Sadistic obsession!",
                                 SfxId::insanityRising);
                  obsessions[int(Obsession::sadism)] = true;
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
            Popup::showMsg(
              msg, true, "Haunted by shadows!", SfxId::insanityRising);

            const int NR_SHADOWS_LOWER = 1;
            const int NR_SHADOWS_UPPER =
              getConstrInRange(2, (Map::dlvl + 1) / 2, 6);
            const int NR_SHADOWS =
              Rnd::range(NR_SHADOWS_LOWER, NR_SHADOWS_UPPER);

            ActorFactory::summonMonsters(
              pos, vector<ActorId>(NR_SHADOWS, actor_shadow), true);

            return;
          }
        } break;

        case 8: {
          msg += "I find myself in a peculiar detached daze, "
                 "a tranced state of mind. I struggle to recall "
                 "where I am, or what I'm doing.";
          Popup::showMsg(msg, true, "Confusion!", SfxId::insanityRising);

          propHandler_->tryApplyProp(new PropConfused(propTurnsStd));

          return;
        } break;

        default: {} break;
      }
    }
  }
}

void Player::addTmpShockFromFeatures() {
  if(
    Map::cells[pos.x][pos.y].isDark &&
    Map::cells[pos.x][pos.y].isLight == false) {
    shockTmp_ += 20;
  }

  for(int dy = -1; dy <= 1; dy++) {
    const int Y = pos.y + dy;
    for(int dx = -1; dx <= 1; dx++) {
      const int X = pos.x + dx;
      if(Utils::isPosInsideMap(Pos(X, Y))) {
        Cell& cell = Map::cells[pos.x + dx][pos.y + dy];
        const Feature* const f = cell.featureStatic;
        shockTmp_ += f->getShockWhenAdjacent();
      }
    }
  }
  shockTmp_ = min(99.0, shockTmp_);
}

bool Player::isStandingInOpenSpace() const {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);
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
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);
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
  vector<Actor*> SpottedEnemies;
  getSpottedEnemies(SpottedEnemies);

  const int ROLL = Rnd::percentile();
  //Phobia vs creature type?
  if(ROLL < 10) {
    for(unsigned int i = 0; i < SpottedEnemies.size(); i++) {
      const ActorDataT& monsterData = SpottedEnemies.at(0)->getData();
      if(monsterData.isCanine && phobias[int(Phobia::dog)]) {
        Log::addMsg("I am plagued by my canine phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(propTurnsSpecific, Rnd::dice(1, 6)));
        return;
      }
      if(monsterData.isRat && phobias[int(Phobia::rat)]) {
        Log::addMsg("I am plagued by my rat phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(propTurnsSpecific, Rnd::dice(1, 6)));
        return;
      }
      if(monsterData.isUndead && phobias[int(Phobia::undead)]) {
        Log::addMsg("I am plagued by my phobia of the dead!");
        propHandler_->tryApplyProp(
          new PropTerrified(propTurnsSpecific, Rnd::dice(1, 6)));
        return;
      }
      if(monsterData.isSpider && phobias[int(Phobia::spider)]) {
        Log::addMsg("I am plagued by my spider phobia!");
        propHandler_->tryApplyProp(
          new PropTerrified(propTurnsSpecific, Rnd::dice(1, 6)));
        return;
      }
    }
  }
  if(ROLL < 5) {
    if(phobias[int(Phobia::openPlace)]) {
      if(isStandingInOpenSpace()) {
        Log::addMsg("I am plagued by my phobia of open places!");
        propHandler_->tryApplyProp(
          new PropTerrified(propTurnsSpecific, Rnd::dice(1, 6)));
        return;
      }
    }

    if(phobias[int(Phobia::closedPlace)]) {
      if(isStandingInCrampedSpace()) {
        Log::addMsg("I am plagued by my phobia of closed places!");
        propHandler_->tryApplyProp(
          new PropTerrified(propTurnsSpecific, Rnd::dice(1, 6)));
        return;
      }
    }
  }
}

void Player::updateColor() {
  if(deadState != ActorDeadState::alive) {
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

  const int CUR_SHOCK = shock_ + shockTmp_;
  if(CUR_SHOCK >= 75) {
    clr_ = clrMagenta;
    return;
  }

  clr_ = data_->color;
}

void Player::onActorTurn() {
  Renderer::drawMapAndInterface();

  resetPermShockTakenCurTurn();

  if(deadState != ActorDeadState::alive) {return;}

  //If player dropped item, check if should go back to inventory screen
  vector<Actor*> spottedEnemies;
  getSpottedEnemies(spottedEnemies);
  if(spottedEnemies.empty()) {
    const InvScrId invScreen = InvHandling::screenToOpenAfterDrop;
    if(invScreen != InvScrId::endOfInventoryScreens) {
      switch(invScreen) {
        case InvScrId::backpack: {
          InvHandling::runBrowseInventory();
        } break;

        case InvScrId::use: {
          InvHandling::runUseScreen();
        } break;

        case InvScrId::equip: {
          InvHandling::runEquipScreen(InvHandling::equipSlotToOpenAfterDrop);
        } break;

        case InvScrId::slots: {
          InvHandling::runSlotsScreen();
        } break;

        case InvScrId::endOfInventoryScreens: {} break;
      }
      return;
    }
  } else {
    InvHandling::screenToOpenAfterDrop    = InvScrId::endOfInventoryScreens;
    InvHandling::browserPosToSetAfterDrop = 0;
  }

  if(Config::isBotPlaying()) {
    Bot::act();
  } else {
    Input::clearEvents();
    Input::handleMapModeInputUntilFound();
  }
}

void Player::onStandardTurn() {
  shockTmp_ = 0.0;
  addTmpShockFromFeatures();

  // Dynamite
  if(dynamiteFuseTurns > 0) {
    dynamiteFuseTurns--;
    if(dynamiteFuseTurns > 0) {
      string fuseMsg = "***F";
      for(int i = 0; i < dynamiteFuseTurns; i++) {fuseMsg += "Z";}
      fuseMsg += "***";
      Log::addMsg(fuseMsg, clrYellow);
    }
  }
  if(dynamiteFuseTurns == 0) {
    Log::addMsg("The dynamite explodes in my hands!");
    Explosion::runExplosionAt(pos, ExplType::expl);
    updateColor();
    dynamiteFuseTurns = -1;
  }

  //Molotovs
  if(molotovFuseTurns > 0) {
    molotovFuseTurns--;
  }
  if(molotovFuseTurns == 0) {
    Log::addMsg("The Molotov Cocktail explodes in my hands!");
    molotovFuseTurns = -1;
    updateColor();
    Explosion::runExplosionAt(
      pos, ExplType::applyProp, ExplSrc::misc, 0, SfxId::explosionMolotov,
      new PropBurning(propTurnsStd));
  }

  //Flare
  if(flareFuseTurns > 0) {
    flareFuseTurns--;
  }
  if(flareFuseTurns == 0) {
    Log::addMsg("The flare is extinguished.");
    updateColor();
    flareFuseTurns = -1;
  }

  if(activeMedicalBag == NULL) {
    testPhobias();
  }

  //If obsessions are active, raise shock to a minimum level
  for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
    if(obsessions[i]) {
      shock_ = max(double(MIN_SHOCK_WHEN_OBSESSION), shock_);
      break;
    }
  }

  vector<Actor*> spottedEnemies;
  getSpottedEnemies(spottedEnemies);
  double shockFromMonstersCurPlayerTurn = 0.0;
  for(Actor * actor : spottedEnemies) {
    DungeonMaster::onMonsterSpotted(*actor);

    Monster* monster = dynamic_cast<Monster*>(actor);

    monster->playerBecomeAwareOfMe();

    const ActorDataT& data = monster->getData();
    if(data.monsterShockLevel != MonsterShockLevel::none) {
      switch(data.monsterShockLevel) {
        case MonsterShockLevel::unsettling: {
          monster->shockCausedCurrent_ =
            min(monster->shockCausedCurrent_ + 0.05,  1.0);
        } break;
        case MonsterShockLevel::scary: {
          monster->shockCausedCurrent_ =
            min(monster->shockCausedCurrent_ + 0.1,   1.0);
        } break;
        case MonsterShockLevel::terrifying: {
          monster->shockCausedCurrent_ =
            min(monster->shockCausedCurrent_ + 0.5,   2.0);
        } break;
        case MonsterShockLevel::mindShattering: {
          monster->shockCausedCurrent_ =
            min(monster->shockCausedCurrent_ + 0.75,  3.0);
        } break;
        default: {} break;
      }
      if(shockFromMonstersCurPlayerTurn < 2.5) {
        incrShock(int(floor(monster->shockCausedCurrent_)),
                  ShockSrc::seeMonster);
        shockFromMonstersCurPlayerTurn += monster->shockCausedCurrent_;
      }
    }
  }

  //Some shock is taken every Xth turn
  int loseNTurns = 12;
  if(PlayerBon::getBg() == Bg::rogue) loseNTurns *= 2;
  const int TURN = GameTime::getTurn();
  if(TURN % loseNTurns == 0 && TURN > 1) {
    if(Rnd::oneIn(850)) {
      if(Rnd::coinToss()) {
        Popup::showMsg("I have a bad feeling about this...", true);
      } else {
        Popup::showMsg("A chill runs down my spine...", true);
      }
      incrShock(ShockValue::shockValue_heavy, ShockSrc::misc);
      Renderer::drawMapAndInterface();
    } else {
      if(Map::dlvl != 0) {
        incrShock(1, ShockSrc::time);
      }
    }
  }

  //Take sanity hit from high shock?
  if(getShockTotal() >= 100) {
    nrTurnsUntilIns_ = nrTurnsUntilIns_ < 0 ? 3 : nrTurnsUntilIns_ - 1;
    if(nrTurnsUntilIns_ > 0) {
      Log::addMsg("I feel my sanity slipping...", clrMsgWarning, true,
                  true);
    } else {
      nrTurnsUntilIns_ = -1;
      incrInsanity();
      if(deadState == ActorDeadState::alive) {
        GameTime::actorDidAct();
      }
      return;
    }
  } else {
    nrTurnsUntilIns_ = -1;
  }

  for(Actor * actor : GameTime::actors_) {
    if(actor != this) {
      if(actor->deadState == ActorDeadState::alive) {

        Monster& monster = *dynamic_cast<Monster*>(actor);
        const bool IS_MONSTER_SEEN = isSeeingActor(*actor, NULL);
        if(IS_MONSTER_SEEN) {
          if(monster.messageMonsterInViewPrinted == false) {
            if(activeMedicalBag != NULL || waitTurnsLeft > 0) {
              Log::addMsg(actor->getNameA() + " comes into my view.",
                          clrWhite, true);
            }
            monster.messageMonsterInViewPrinted = true;
          }
        } else {
          monster.messageMonsterInViewPrinted = false;

          //Is the monster sneaking? Try to spot it
          if(Map::cells[monster.pos.x][monster.pos.y].isSeenByPlayer) {
            if(monster.isStealth) {
              if(isSpottingHiddenActor(monster)) {
                monster.isStealth = false;
                updateFov();
                Renderer::drawMapAndInterface();
                Log::addMsg("I spot " + monster.getNameA() + "!",
                            clrMsgWarning, true, true);
              }
            }
          }
        }
      }
    }
  }

  const int DECR_ABOVE_MAX_N_TURNS = 7;
  if(getHp() > getHpMax(true)) {
    if(GameTime::getTurn() % DECR_ABOVE_MAX_N_TURNS == 0) {hp_--;}
  }
  if(getSpi() > getSpiMax()) {
    if(GameTime::getTurn() % DECR_ABOVE_MAX_N_TURNS == 0) {spi_--;}
  }

  vector<PropId> props;
  propHandler_->getAllActivePropIds(props);

  if(activeMedicalBag == NULL) {
    if(find(props.begin(), props.end(), propPoisoned) == props.end()) {
      int nrWounds = 0;
      Prop* const propWnd = propHandler_->getProp(propWound, PropSrc::applied);
      if(propWnd != NULL) {
        nrWounds = dynamic_cast<PropWound*>(propWnd)->getNrWounds();
      }

      const bool IS_RAPID_REC     = PlayerBon::hasTrait(Trait::rapidRecoverer);
      const bool IS_SURVIVALIST   = PlayerBon::hasTrait(Trait::survivalist);

      //Survivalist trait halves the penalty form wounds
      const int WOUND_TURNS_DIV  = IS_SURVIVALIST ? 2 : 1;

      const int REGEN_N_TURN = (IS_SURVIVALIST ? 4 : (IS_RAPID_REC ? 7 : 10))
                               + ((nrWounds * 5) / WOUND_TURNS_DIV);

      if((TURN / REGEN_N_TURN) * REGEN_N_TURN == TURN && TURN > 1) {
        if(getHp() < getHpMax(true)) {
          hp_++;
        }
      }
    }

    if(
      propHandler_->allowSee() &&
      find(props.begin(), props.end(), propConfused) == props.end()) {

      const int R = PlayerBon::hasTrait(Trait::perceptive) ? 3 :
                    (PlayerBon::hasTrait(Trait::observant) ? 2 : 1);

      int x0 = max(0, pos.x - R);
      int y0 = max(0, pos.y - R);
      int x1 = min(MAP_W - 1, pos.x + R);
      int y1 = min(MAP_H - 1, pos.y + R);

      for(int y = y0; y <= y1; y++) {
        for(int x = x0; x <= x1; x++) {
          if(Map::cells[x][y].isSeenByPlayer) {
            Feature* f = Map::cells[x][y].featureStatic;

            if(f->getId() == FeatureId::trap) {
              dynamic_cast<Trap*>(f)->playerTrySpotHidden();
            }
            if(f->getId() == FeatureId::door) {
              dynamic_cast<Door*>(f)->playerTrySpotHidden();
            }
          }
        }
      }
    }
  }

  if(activeMedicalBag != NULL) {
    activeMedicalBag->continueAction();
  }

  if(waitTurnsLeft > 0) {
    waitTurnsLeft--;
    GameTime::actorDidAct();
  }
}

void Player::interruptActions() {
  Renderer::drawMapAndInterface();

  InvHandling::screenToOpenAfterDrop    = InvScrId::endOfInventoryScreens;
  InvHandling::browserPosToSetAfterDrop = 0;

  //Abort searching
  if(waitTurnsLeft > 0) {
    Log::addMsg("I stop waiting.", clrWhite);
    Renderer::drawMapAndInterface();
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
  Renderer::drawMapAndInterface();
}

void Player::hearSound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                       const Dir dirToOrigin,
                       const int PERCENT_AUDIBLE_DISTANCE) {
  const SfxId sfx = snd.getSfx();

  const string& msg = snd.getMsg();
  const bool HAS_SND_MSG = msg.empty() == false && msg != " ";

  if(HAS_SND_MSG) {Log::addMsg(msg, clrWhite);}

  //Play audio after message to ensure synch between audio and animation
  //If origin is hidden, we only play the sound if there is a message
  if(HAS_SND_MSG || IS_ORIGIN_SEEN_BY_PLAYER) {
    Audio::play(sfx, dirToOrigin, PERCENT_AUDIBLE_DISTANCE);
  }

  if(HAS_SND_MSG) {
    Actor* const actorWhoMadeSnd = snd.getActorWhoMadeSound();
    if(actorWhoMadeSnd != NULL && actorWhoMadeSnd != this) {
      dynamic_cast<Monster*>(actorWhoMadeSnd)->playerBecomeAwareOfMe();
    }
  }
}

void Player::moveDir(Dir dir) {
  if(deadState == ActorDeadState::alive) {

    propHandler_->changeMoveDir(pos, dir);

    //Trap affects leaving?
    if(dir != Dir::center) {
      Feature* f = Map::cells[pos.x][pos.y].featureStatic;
      if(f->getId() == FeatureId::trap) {
        trace << "Player: Standing on trap, check if affects move" << endl;
        dir = dynamic_cast<Trap*>(f)->actorTryLeave(*this, dir);
      }
    }

    bool isFreeTurn = false;;

    const Pos dest(pos + DirUtils::getOffset(dir));

    if(dir != Dir::center) {
      //Attack?
      Actor* const actorAtDest = Utils::getActorAtPos(dest);
      if(actorAtDest != NULL) {
        if(propHandler_->allowAttackMelee(true)) {
          bool hasMeleeWeapon = false;
          Item* const item = inv_->getItemInSlot(SlotId::wielded);
          if(item != NULL) {
            Weapon* const weapon = dynamic_cast<Weapon*>(item);
            if(weapon->getData().isMeleeWeapon) {
              if(Config::isRangedWpnMeleeePrompt() &&
                  isSeeingActor(*actorAtDest, NULL)) {
                if(weapon->getData().isRangedWeapon) {
                  const string wpnName =
                    ItemData::getItemRef(*weapon, ItemRefType::a);
                  Log::addMsg(
                    "Attack " + actorAtDest->getNameThe() +
                    " with " + wpnName + "? (y/n)", clrWhiteHigh);
                  Renderer::drawMapAndInterface();
                  if(Query::yesOrNo() == YesNoAnswer::no) {
                    Log::clearLog();
                    Renderer::drawMapAndInterface();
                    return;
                  }
                }
              }
              Attack::melee(*this, *weapon, *actorAtDest);
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
      vector<PropId> props;
      getPropHandler().getAllActivePropIds(props);
      Cell& cell = Map::cells[dest.x][dest.y];
      bool isFeaturesAllowMove = cell.featureStatic->canMove(props);

      vector<FeatureMob*> featureMobs;
      GameTime::getFeatureMobsAtPos(dest, featureMobs);

      if(isFeaturesAllowMove) {
        for(FeatureMob * m : featureMobs) {
          if(m->canMove(props) == false) {
            isFeaturesAllowMove = false;
            break;
          }
        }
      }

      if(isFeaturesAllowMove) {
        //Encumbrance
        const int ENC = getEncPercent();
        if(ENC >= ENC_IMMOBILE_LVL) {
          Log::addMsg("I am too encumbered to move!");
          Renderer::drawMapAndInterface();
          return;
        } else if(ENC >= 100) {
          Log::addMsg("I stagger.", clrMsgWarning);
          propHandler_->tryApplyProp(new PropWaiting(propTurnsStd));
        }

        pos = dest;

        const int FREE_MOVE_EVERY_N_TURN =
          PlayerBon::hasTrait(Trait::mobile)     ? 2 :
          PlayerBon::hasTrait(Trait::lithe)      ? 4 :
          PlayerBon::hasTrait(Trait::dexterous)  ? 5 : 0;

        if(FREE_MOVE_EVERY_N_TURN > 0) {
          if(nrMovesUntilFreeAction_ == -1) {
            nrMovesUntilFreeAction_ = FREE_MOVE_EVERY_N_TURN - 2;
          } else if(nrMovesUntilFreeAction_ == 0) {
            nrMovesUntilFreeAction_ = FREE_MOVE_EVERY_N_TURN - 1;
            isFreeTurn = true;
          } else {
            nrMovesUntilFreeAction_--;
          }
        }

        //Print message if walking on item
        Item* const item = Map::cells[pos.x][pos.y].item;
        if(item != NULL) {
          string message = propHandler_->allowSee() == false ?
                           "I feel here: " : "I see here: ";
          message += ItemData::getItemInterfaceRef(*item, true);
          Log::addMsg(message + ".");
        }
      }

      //Note: bump() prints block messages.
      for(FeatureMob * m : featureMobs) {m->bump(*this);}

      Map::cells[dest.x][dest.y].featureStatic->bump(*this);
    }

    if(pos == dest) {
      GameTime::actorDidAct(isFreeTurn);
      return;
    }
  }
}

void Player::autoMelee() {
  if(target != NULL) {
    if(Utils::isPosAdj(pos, target->pos, false)) {
      if(isSeeingActor(*target, NULL)) {
        moveDir(DirUtils::getDir(target->pos - pos));
        return;
      }
    }
  }

  //If this line reached, there is no adjacent current target.
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      if(dx != 0 || dy != 0) {
        Actor* const actor = Utils::getActorAtPos(pos + Pos(dx, dy));
        if(actor != NULL) {
          if(isSeeingActor(*actor, NULL)) {
            target = actor;
            moveDir(DirUtils::getDir(Pos(dx, dy)));
            return;
          }
        }
      }
    }
  }
}

void Player::kick(Actor& actorToKick) {
  Weapon* kickWeapon = NULL;

  const ActorDataT& d = actorToKick.getData();

  if(d.actorSize == actorSize_floor && (d.isSpider || d.isRat)) {
    kickWeapon =
      dynamic_cast<Weapon*>(ItemFactory::spawnItem(ItemId::playerStomp));
  } else {
    kickWeapon =
      dynamic_cast<Weapon*>(ItemFactory::spawnItem(ItemId::playerKick));
  }
  Attack::melee(*this, *kickWeapon, actorToKick);
  delete kickWeapon;
}

void Player::punch(Actor& actorToPunch) {
  //Spawn a temporary punch weapon to attack with
  Weapon* punchWeapon =
    dynamic_cast<Weapon*>(ItemFactory::spawnItem(ItemId::playerPunch));
  Attack::melee(*this, *punchWeapon, actorToPunch);
  delete punchWeapon;
}

void Player::addLight_(bool light[MAP_W][MAP_H]) const {
  bool isUsingLightGivingItemSmall  = false;              //3x3 cells
  bool isUsingLightGivingItemNormal = flareFuseTurns > 0;

  vector<Item*>& generalItems = inv_->getGeneral();
  for(Item * const item : generalItems) {
    if(item->getData().id == ItemId::electricLantern) {
      DeviceLantern* const lantern = dynamic_cast<DeviceLantern*>(item);
      LanternLightSize lightSize = lantern->getCurLightSize();
      if(lightSize == LanternLightSize::small) {
        isUsingLightGivingItemSmall = true;
      } else if(lightSize == LanternLightSize::normal) {
        isUsingLightGivingItemNormal = true;
        break;
      }
    }
  }

  if(isUsingLightGivingItemNormal) {
    bool myLight[MAP_W][MAP_H];
    Utils::resetArray(myLight, false);
    const int RADI = FOV_STD_RADI_INT;
    Pos x0y0(max(0, pos.x - RADI), max(0, pos.y - RADI));
    Pos x1y1(min(MAP_W - 1, pos.x + RADI), min(MAP_H - 1, pos.y + RADI));

    bool visionBlockers[MAP_W][MAP_H];
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      for(int x = x0y0.x; x <= x1y1.x; x++) {
        const FeatureStatic* const f = Map::cells[x][y].featureStatic;
        visionBlockers[x][y] = f->isVisionPassable() == false;
      }
    }

    Fov::runFovOnArray(visionBlockers, pos, myLight, false);
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      for(int x = x0y0.x; x <= x1y1.x; x++) {
        if(myLight[x][y]) {
          light[x][y] = true;
        }
      }
    }
  } else if(isUsingLightGivingItemSmall) {
    for(int y = pos.y - 1; y <= pos.y + 1; y++) {
      for(int x = pos.x - 1; x <= pos.x + 1; x++) {
        light[x][y] = true;
      }
    }
  }
}

void Player::updateFov() {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Map::cells[x][y].isSeenByPlayer = false;
    }
  }

  if(propHandler_->allowSee()) {
    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blockers);
    Fov::runPlayerFov(blockers, pos);
    Map::cells[pos.x][pos.y].isSeenByPlayer = true;
  }

  if(propHandler_->allowSee()) {FOVhack();}

  if(Init::isCheatVisionEnabled) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        Map::cells[x][y].isSeenByPlayer = true;
      }
    }
  }

  //Explore
  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      Cell& cell = Map::cells[x][y];
      const bool IS_BLOCKING = CellPred::BlocksMoveCmn(false).check(cell);
      //Do not explore dark floor cells
      if(cell.isSeenByPlayer && (cell.isDark == false || IS_BLOCKING)) {
        cell.isExplored = true;
      }
    }
  }
}

void Player::FOVhack() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(visionBlockers[x][y] && blockers[x][y]) {
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            const Pos adj(x + dx, y + dy);
            if(Utils::isPosInsideMap(adj)) {
              const Cell& adjCell = Map::cells[adj.x][adj.y];
              if(
                adjCell.isSeenByPlayer &&
                (adjCell.isDark == false || adjCell.isLight) &&
                blockers[adj.x][adj.y] == false) {
                Map::cells[x][y].isSeenByPlayer = true;
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
