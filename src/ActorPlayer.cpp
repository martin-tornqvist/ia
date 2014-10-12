#include "ActorPlayer.h"

#include "Init.h"
#include "Render.h"
#include "Audio.h"
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
#include "FeatureMob.h"
#include "Query.h"
#include "Attack.h"
#include "Fov.h"
#include "ItemFactory.h"
#include "ActorFactory.h"
#include "PlayerBon.h"
#include "Inventory.h"
#include "InventoryHandling.h"
#include "PlayerSpellsHandling.h"
#include "Bot.h"
#include "Input.h"
#include "MapParsing.h"
#include "Properties.h"
#include "ItemDevice.h"
#include "ItemScroll.h"
#include "ItemPotion.h"

using namespace std;

const int MIN_SHOCK_WHEN_OBSESSION = 35;

Player::Player() :
  Actor(),
  activeMedicalBag(nullptr),
  waitTurnsLeft(-1),
  activeExplosive(nullptr),
  target(nullptr),
  ins_(0),
  shock_(0.0),
  shockTmp_(0.0),
  permShockTakenCurTurn_(0.0),
  nrMovesUntilFreeAction_(-1),
  nrTurnsUntilIns_(-1),
  CARRY_WEIGHT_BASE_(450) {}

Player::~Player() {
  if(activeExplosive) {delete activeExplosive;}
}

void Player::mkStartItems() {
  data_->abilityVals.reset();

  for(int i = 0; i < int(Phobia::END); ++i)     {phobias[i]     = false;}
  for(int i = 0; i < int(Obsession::END); ++i)  {obsessions[i]  = false;}

  int nrCartridges  = 2;
  int nrDynamite    = 2;
  int nrMolotov     = 2;
  int nrThrKnives   = 6;

  //------------------------------------------------------- BACKGROUND SPECIFIC SETUP
  const auto bg = PlayerBon::getBg();

  if(bg == Bg::occultist) {

    //Occultist starts with zero explosives and throwing knives.
    //(They are not so thematically fitting for this background.)
    nrDynamite    = 0;
    nrMolotov     = 0;
    nrThrKnives   = 0;

    //Occultist starts with a scroll of Darkbolt, and one other random scroll.
    //(Both are identified.)
    Item* scroll = ItemFactory::mk(ItemId::scrollDarkbolt);
    static_cast<Scroll*>(scroll)->identify(true);
    inv_->putInGeneral(scroll);
    while(true) {
      scroll = ItemFactory::mkRandomScrollOrPotion(true, false);

      SpellId id          = scroll->getData().spellCastFromScroll;
      Spell* const spell  = SpellHandling::mkSpellFromId(id);
      const bool IS_AVAIL = spell->isAvailForPlayer();
      delete spell;

      if(IS_AVAIL && id != SpellId::darkbolt) {
        static_cast<Scroll*>(scroll)->identify(true);
        inv_->putInGeneral(scroll);
        break;
      }
    }

    //Occultist starts with a few potions (identified).
    const int NR_POTIONS = 2;
    for(int i = 0; i < NR_POTIONS; ++i) {
      Item* const potion = ItemFactory::mkRandomScrollOrPotion(false, true);
      static_cast<Potion*>(potion)->identify(true);
      inv_->putInGeneral(potion);
    }
  }

  if(bg == Bg::rogue) {
    //Rogue starts with extra throwing knives
    nrThrKnives += 6;

    //Rogue starts with a dagger
    inv_->slots_[int(SlotId::wielded)].item = ItemFactory::mk(ItemId::dagger);

    //Rogue starts with some iron spikes (useful tool)
    inv_->putInGeneral(ItemFactory::mk(ItemId::ironSpike, 8));
  }

  if(bg == Bg::warVet) {
    //War Veteran starts with some smoke grenades and a gas mask
    inv_->putInGeneral(ItemFactory::mk(ItemId::smokeGrenade, 4));
    inv_->putInGeneral(ItemFactory::mk(ItemId::gasMask));
  }

  //------------------------------------------------------- GENERAL SETUP
  //Randomize a melee weapon if not already wielding one.
  if(!inv_->slots_[int(SlotId::wielded)].item) {
    const int WEAPON_CHOICE = Rnd::range(1, 5);
    auto weaponId = ItemId::dagger;
    switch(WEAPON_CHOICE) {
      case 1:   weaponId = ItemId::dagger;   break;
      case 2:   weaponId = ItemId::hatchet;  break;
      case 3:   weaponId = ItemId::hammer;   break;
      case 4:   weaponId = ItemId::machete;  break;
      case 5:   weaponId = ItemId::axe;      break;
      default:  weaponId = ItemId::dagger;   break;
    }
    inv_->putInSlot(SlotId::wielded, ItemFactory::mk(weaponId));
  }

  inv_->putInSlot(SlotId::wieldedAlt, ItemFactory::mk(ItemId::pistol));

  for(int i = 0; i < nrCartridges; ++i) {
    inv_->putInGeneral(ItemFactory::mk(ItemId::pistolClip));
  }

  if(nrDynamite > 0) {
    inv_->putInGeneral(ItemFactory::mk(ItemId::dynamite,  nrDynamite));
  }
  if(nrMolotov > 0) {
    inv_->putInGeneral(ItemFactory::mk(ItemId::molotov,   nrMolotov));
  }

  if(nrThrKnives > 0) {
    inv_->putInSlot(SlotId::thrown, ItemFactory::mk(ItemId::throwingKnife, nrThrKnives));
  }

  inv_->putInSlot(SlotId::body, ItemFactory::mk(ItemId::armorLeatherJacket));

  inv_->putInGeneral(ItemFactory::mk(ItemId::electricLantern));
  inv_->putInGeneral(ItemFactory::mk(ItemId::medicalBag));
}

void Player::storeToSaveLines(vector<string>& lines) const {
  lines.push_back(toStr(propHandler_->appliedProps_.size()));
  for(Prop* prop : propHandler_->appliedProps_) {
    lines.push_back(toStr(prop->getId()));
    lines.push_back(toStr(prop->turnsLeft_));
    prop->storeToSaveLines(lines);
  }

  lines.push_back(toStr(ins_));
  lines.push_back(toStr(int(shock_)));
  lines.push_back(toStr(hp_));
  lines.push_back(toStr(hpMax_));
  lines.push_back(toStr(spi_));
  lines.push_back(toStr(spiMax_));
  lines.push_back(toStr(pos.x));
  lines.push_back(toStr(pos.y));

  for(int i = 0; i < int(AbilityId::END); ++i) {
    lines.push_back(toStr(data_->abilityVals.getRawVal(AbilityId(i))));
  }

  for(int i = 0; i < int(Phobia::END); ++i) {
    lines.push_back(phobias[i] == 0 ? "0" : "1");
  }
  for(int i = 0; i < int(Obsession::END); ++i) {
    lines.push_back(obsessions[i] == 0 ? "0" : "1");
  }
}

void Player::setupFromSaveLines(vector<string>& lines) {
  const int NR_PROPS = toInt(lines.front());
  lines.erase(begin(lines));
  for(int i = 0; i < NR_PROPS; ++i) {
    const auto id = PropId(toInt(lines.front()));
    lines.erase(begin(lines));
    const int NR_TURNS = toInt(lines.front());
    lines.erase(begin(lines));
    auto* const prop = propHandler_->mkProp(
                         id, PropTurns::specific, NR_TURNS);
    propHandler_->tryApplyProp(prop, true, true, true, true);
    prop->setupFromSaveLines(lines);
  }

  ins_ = toInt(lines.front());
  lines.erase(begin(lines));
  shock_ = double(toInt(lines.front()));
  lines.erase(begin(lines));
  hp_ = toInt(lines.front());
  lines.erase(begin(lines));
  hpMax_ = toInt(lines.front());
  lines.erase(begin(lines));
  spi_ = toInt(lines.front());
  lines.erase(begin(lines));
  spiMax_ = toInt(lines.front());
  lines.erase(begin(lines));
  pos.x = toInt(lines.front());
  lines.erase(begin(lines));
  pos.y = toInt(lines.front());
  lines.erase(begin(lines));

  for(int i = 0; i < int(AbilityId::END); ++i) {
    data_->abilityVals.setVal(AbilityId(i), toInt(lines.front()));
    lines.erase(begin(lines));
  }

  for(int i = 0; i < int(Phobia::END); ++i) {
    phobias[i] = lines.front() == "0" ? false : true;
    lines.erase(begin(lines));
  }
  for(int i = 0; i < int(Obsession::END); ++i) {
    obsessions[i] = lines.front() == "0" ? false : true;
    lines.erase(begin(lines));
  }
}

void Player::hit_(int& dmg) {
  (void)dmg;

  if(!obsessions[int(Obsession::masochism)]) {incrShock(1, ShockSrc::misc);}

  Render::drawMapAndInterface();
}

int Player::getEncPercent() const {
  const int TOTAL_W = inv_->getTotalItemWeight();
  const int MAX_W   = getCarryWeightLmt();
  return int((double(TOTAL_W) / double(MAX_W)) * 100.0);
}

int Player::getCarryWeightLmt() const {
  const bool IS_TOUGH         = PlayerBon::hasTrait(Trait::tough);
  const bool IS_RUGGED        = PlayerBon::hasTrait(Trait::rugged);
  const bool IS_STRONG_BACKED = PlayerBon::hasTrait(Trait::strongBacked);

  vector<PropId> props;
  propHandler_->getAllActivePropIds(props);
  const bool IS_WEAKENED = find(begin(props), end(props), propWeakened) != end(props);

  const int CARRY_WEIGHT_MOD = (IS_TOUGH         * 10) +
                               (IS_RUGGED        * 10) +
                               (IS_STRONG_BACKED * 30) -
                               (IS_WEAKENED      * 15);

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
    case ShockSrc::END: {} break;
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
  for(int i = 0; i < int(Obsession::END); ++i) {
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
  TRACE << "Increasing insanity" << endl;
  string msg = getInsanity() < 100 ? "Insanity draws nearer... " : "";

  if(!Config::isBotPlaying()) {
    const int INS_INCR = 6;
    ins_ += INS_INCR;
  }

  restoreShock(70, false);

  updateClr();
  Render::drawMapAndInterface();

  if(getInsanity() >= 100) {
    msg += "My mind can no longer withstand what it has grasped. "
           "I am hopelessly lost.";
    Popup::showMsg(msg, true, "Complete insanity!", SfxId::insanityRise);
    die(true, false, false);
  } else {
    bool playerSeeShockingMonster = false;
    vector<Actor*> seenFoes;
    getSeenFoes(seenFoes);
    for(Actor* actor : seenFoes) {
      const ActorDataT& def = actor->getData();
      if(def.monsterShockLvl != MonsterShockLvl::none) {
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
            Popup::showMsg(msg, true, "Screaming!", SfxId::insanityRise);
            Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos, this,
                    SndVol::high, AlertsMonsters::yes);
            SndEmit::emitSnd(snd);
            return;
          }
        } break;

        case 2: {
          msg += "I find myself babbling incoherently.";
          Popup::showMsg(msg, true, "Babbling!", SfxId::insanityRise);
          const string playerName = getNameThe();
          for(int i = Rnd::range(3, 5); i > 0; --i) {
            const string phrase = Cultist::getCultistPhrase();
            Log::addMsg(playerName + ": " + phrase);
          }
          Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos, this,
                  SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          return;
        } break;

        case 3: {
          msg += "I struggle to not fall into a stupor.";
          Popup::showMsg(msg, true, "Fainting!", SfxId::insanityRise);
          propHandler_->tryApplyProp(new PropFainted(PropTurns::std));
          return;
        } break;

        case 4: {
          msg += "I laugh nervously.";
          Popup::showMsg(msg, true, "HAHAHA!", SfxId::insanityRise);
          Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos, this,
                  SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          return;
        } break;

        case 5: {
          vector<PropId> props;
          propHandler_->getAllActivePropIds(props);

          if(find(begin(props), end(props), propRFear) != end(props)) {

            if(ins_ > 5) {
              //There is a limit to the number of phobias you can have
              int phobiasActive = 0;
              for(int i = 0; i < int(Phobia::END); ++i) {
                if(phobias[i]) {phobiasActive++;}
              }
              if(phobiasActive < 2) {
                if(Rnd::coinToss()) {
                  if(!seenFoes.empty()) {
                    const int M_ROLL = Rnd::range(0, seenFoes.size() - 1);
                    const ActorDataT& monsterData = seenFoes.at(M_ROLL)->getData();
                    if(monsterData.isRat && !phobias[int(Phobia::rat)]) {
                      msg += "I am afflicted by Murophobia. "
                             "Rats suddenly seem terrifying.";
                      Popup::showMsg(msg, true, "Murophobia!", SfxId::insanityRise);
                      phobias[int(Phobia::rat)] = true;
                      return;
                    }
                    if(monsterData.isSpider && !phobias[int(Phobia::spider)]) {
                      msg += "I am afflicted by Arachnophobia. "
                             "Spiders suddenly seem terrifying.";
                      Popup::showMsg(msg, true, "Arachnophobia!", SfxId::insanityRise);
                      phobias[int(Phobia::spider)] = true;
                      return;
                    }
                    if(monsterData.isCanine && !phobias[int(Phobia::dog)]) {
                      msg += "I am afflicted by Cynophobia. "
                             "Dogs suddenly seem terrifying.";
                      Popup::showMsg(msg, true, "Cynophobia!", SfxId::insanityRise);
                      phobias[int(Phobia::dog)] = true;
                      return;
                    }
                    if(monsterData.isUndead && !phobias[int(Phobia::undead)]) {
                      msg += "I am afflicted by Necrophobia. "
                             "The undead suddenly seem much more terrifying.";
                      Popup::showMsg(msg, true, "Necrophobia!", SfxId::insanityRise);
                      phobias[int(Phobia::undead)] = true;
                      return;
                    }
                  }
                } else {
                  if(Rnd::coinToss()) {
                    if(isStandingInOpenSpace()) {
                      if(!phobias[int(Phobia::openPlace)]) {
                        msg += "I am afflicted by Agoraphobia. "
                               "Open places suddenly seem terrifying.";
                        Popup::showMsg(msg, true, "Agoraphobia!", SfxId::insanityRise);
                        phobias[int(Phobia::openPlace)] = true;
                        return;
                      }
                    }
                    if(isStandingInCrampedSpace()) {
                      if(!phobias[int(Phobia::closedPlace)]) {
                        msg += "I am afflicted by Claustrophobia. "
                               "Confined places suddenly seem terrifying.";
                        Popup::showMsg(msg, true, "Claustrophobia!", SfxId::insanityRise);
                        phobias[int(Phobia::closedPlace)] = true;
                        return;
                      }
                    }
                  } else {
                    if(Map::dlvl >= 5) {
                      if(!phobias[int(Phobia::deepPlaces)]) {
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
          if(ins_ > 20) {
            int obsessionsActive = 0;
            for(int i = 0; i < int(Obsession::END); ++i) {
              if(obsessions[i]) {obsessionsActive++;}
            }
            if(obsessionsActive == 0) {
              const Obsession obsession =
                (Obsession)(Rnd::range(0, int(Obsession::END) - 1));
              switch(obsession) {
                case Obsession::masochism: {
                  msg +=
                    "To my alarm, I find myself encouraged by the sensation of "
                    "pain. Physical suffering does not frighten me at all. "
                    "However, my depraved mind can never find complete peace "
                    "(no shock from taking damage, but shock cannot go below "
                    + toStr(MIN_SHOCK_WHEN_OBSESSION) + "%).";
                  Popup::showMsg(msg, true, "Masochistic obsession!",
                                 SfxId::insanityRise);
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
                  Popup::showMsg(msg, true, "Sadistic obsession!", SfxId::insanityRise);
                  obsessions[int(Obsession::sadism)] = true;
                  return;
                } break;
                default: {} break;
              }
            }
          }
        } break;

        case 7: {
          if(ins_ > 8) {
            msg += "The shadows are closing in on me!";
            Popup::showMsg(msg, true, "Haunted by shadows!", SfxId::insanityRise);
            const int NR_SHADOWS_LOWER  = 1;
            const int NR_SHADOWS_UPPER  = getConstrInRange(2, (Map::dlvl + 1) / 2, 6);
            const int NR                = Rnd::range(NR_SHADOWS_LOWER, NR_SHADOWS_UPPER);
            ActorFactory::summonMonsters(pos, vector<ActorId>(NR, ActorId::shadow), true);
            return;
          }
        } break;

        case 8: {
          msg += "I find myself in a peculiar detached daze, "
                 "a tranced state of mind. I struggle to recall "
                 "where I am, or what I'm doing.";
          Popup::showMsg(msg, true, "Confusion!", SfxId::insanityRise);
          propHandler_->tryApplyProp(new PropConfused(PropTurns::std));
          return;
        } break;

        default: {} break;
      }
    }
  }
}

void Player::addTmpShockFromFeatures() {
  Cell& cell = Map::cells[pos.x][pos.y];

  if(cell.isDark && !cell.isLight) {shockTmp_ += 20;}

  for(int dy = -1; dy <= 1; ++dy) {
    const int Y = pos.y + dy;
    for(int dx = -1; dx <= 1; ++dx) {
      const int X = pos.x + dx;
      if(Utils::isPosInsideMap(Pos(X, Y))) {
        shockTmp_ += Map::cells[X][Y].rigid->getShockWhenAdj();
      }
    }
  }
  shockTmp_ = min(99.0, shockTmp_);
}

bool Player::isStandingInOpenSpace() const {
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  for(int y = pos.y - 1; y <= pos.y + 1; ++y) {
    for(int x = pos.x - 1; x <= pos.x + 1; ++x) {
      if(blocked[x][y]) {return false;}
    }
  }
  return true;
}

bool Player::isStandingInCrampedSpace() const {
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  int blockCount = 0;
  for(int y = pos.y - 1; y <= pos.y + 1; ++y) {
    for(int x = pos.x - 1; x <= pos.x + 1; ++x) {
      if(blocked[x][y]) {
        blockCount++;
        if(blockCount >= 6) {return true;}
      }
    }
  }

  return false;
}

void Player::testPhobias() {
  vector<Actor*> seenFoes;
  getSeenFoes(seenFoes);

  const int ROLL = Rnd::percentile();
  //Phobia vs creature type?
  if(ROLL < 10) {
    for(Actor* const actor : seenFoes) {
      const ActorDataT& monsterData = actor->getData();
      if(monsterData.isCanine && phobias[int(Phobia::dog)]) {
        Log::addMsg("I am plagued by my canine phobia!");
        propHandler_->tryApplyProp(new PropTerrified(PropTurns::specific, Rnd::dice(1, 6)));
        return;
      }
      if(monsterData.isRat && phobias[int(Phobia::rat)]) {
        Log::addMsg("I am plagued by my rat phobia!");
        propHandler_->tryApplyProp(new PropTerrified(PropTurns::specific, Rnd::dice(1, 6)));
        return;
      }
      if(monsterData.isUndead && phobias[int(Phobia::undead)]) {
        Log::addMsg("I am plagued by my phobia of the dead!");
        propHandler_->tryApplyProp(new PropTerrified(PropTurns::specific, Rnd::dice(1, 6)));
        return;
      }
      if(monsterData.isSpider && phobias[int(Phobia::spider)]) {
        Log::addMsg("I am plagued by my spider phobia!");
        propHandler_->tryApplyProp(new PropTerrified(PropTurns::specific, Rnd::dice(1, 6)));
        return;
      }
    }
  }
  if(ROLL < 5) {
    if(phobias[int(Phobia::openPlace)]) {
      if(isStandingInOpenSpace()) {
        Log::addMsg("I am plagued by my phobia of open places!");
        propHandler_->tryApplyProp(new PropTerrified(PropTurns::specific, Rnd::dice(1, 6)));
        return;
      }
    }

    if(phobias[int(Phobia::closedPlace)]) {
      if(isStandingInCrampedSpace()) {
        Log::addMsg("I am plagued by my phobia of closed places!");
        propHandler_->tryApplyProp(new PropTerrified(PropTurns::specific, Rnd::dice(1, 6)));
        return;
      }
    }
  }
}

void Player::updateClr() {
  if(deadState != ActorDeadState::alive) {
    clr_ = clrRed;
    return;
  }

  if(propHandler_->changeActorClr(clr_)) {return; }

  if(activeExplosive) {
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
  Render::drawMapAndInterface();

  resetPermShockTakenCurTurn();

  if(deadState != ActorDeadState::alive) {return;}

  //If player dropped item, check if should go back to inventory screen
  vector<Actor*> seenFoes;
  getSeenFoes(seenFoes);
  if(seenFoes.empty()) {
    const auto invScreen = InvHandling::screenToOpenAfterDrop;
    if(invScreen != InvScrId::END) {
      switch(invScreen) {
        case InvScrId::inv: {
          InvHandling::runInvScreen();
        } break;

        case InvScrId::equip: {
          InvHandling::runEquipScreen(*InvHandling::equipSlotToOpenAfterDrop);
        } break;

        case InvScrId::END: {} break;
      }
      return;
    }
  } else {
    InvHandling::screenToOpenAfterDrop    = InvScrId::END;
    InvHandling::browserIdxToSetAfterDrop = 0;
  }

  if(Config::isBotPlaying()) {
    Bot::act();
  } else {
    Input::clearEvents();
    Input::handleMapModeInputUntilFound();
  }
}

void Player::onStdTurn() {
  shockTmp_ = 0.0;
  addTmpShockFromFeatures();

  if(activeExplosive)   {activeExplosive->onStdTurnPlayerHoldIgnited();}

  if(!activeMedicalBag) {testPhobias();}

  //If obsessions are active, raise shock to a minimum level
  for(int i = 0; i < int(Obsession::END); ++i) {
    if(obsessions[i]) {
      shock_ = max(double(MIN_SHOCK_WHEN_OBSESSION), shock_);
      break;
    }
  }

  vector<Actor*> seenFoes;
  getSeenFoes(seenFoes);
  double shockFromMonstersCurPlayerTurn = 0.0;
  for(Actor* actor : seenFoes) {
    DungeonMaster::onMonsterSpotted(*actor);

    Monster* monster = static_cast<Monster*>(actor);

    monster->playerBecomeAwareOfMe();

    const ActorDataT& data = monster->getData();
    if(data.monsterShockLvl != MonsterShockLvl::none) {
      switch(data.monsterShockLvl) {
        case MonsterShockLvl::unsettling: {
          monster->shockCausedCur_ =
            min(monster->shockCausedCur_ + 0.05,  1.0);
        } break;
        case MonsterShockLvl::scary: {
          monster->shockCausedCur_ =
            min(monster->shockCausedCur_ + 0.1,   1.0);
        } break;
        case MonsterShockLvl::terrifying: {
          monster->shockCausedCur_ =
            min(monster->shockCausedCur_ + 0.5,   2.0);
        } break;
        case MonsterShockLvl::mindShattering: {
          monster->shockCausedCur_ =
            min(monster->shockCausedCur_ + 0.75,  3.0);
        } break;
        default: {} break;
      }
      if(shockFromMonstersCurPlayerTurn < 2.5) {
        incrShock(int(floor(monster->shockCausedCur_)), ShockSrc::seeMonster);
        shockFromMonstersCurPlayerTurn += monster->shockCausedCur_;
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
      incrShock(ShockValue::heavy, ShockSrc::misc);
      Render::drawMapAndInterface();
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
      Log::addMsg("I feel my sanity slipping...", clrMsgWarning, true, true);
    } else {
      nrTurnsUntilIns_ = -1;
      incrInsanity();
      if(deadState == ActorDeadState::alive) {GameTime::actorDidAct();}
      return;
    }
  } else {
    nrTurnsUntilIns_ = -1;
  }

  for(Actor* actor : GameTime::actors_) {
    if(actor != this) {
      if(actor->deadState == ActorDeadState::alive) {

        Monster& monster = *static_cast<Monster*>(actor);
        const bool IS_MONSTER_SEEN = isSeeingActor(*actor, nullptr);
        if(IS_MONSTER_SEEN) {
          if(!monster.messageMonsterInViewPrinted) {
            if(activeMedicalBag || waitTurnsLeft > 0) {
              Log::addMsg(actor->getNameA() + " comes into my view.", clrWhite, true);
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
                Render::drawMapAndInterface();
                const string monName = monster.getNameA();
                Log::addMsg("I spot " + monName + "!", clrMsgWarning, true, true);
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

  if(!activeMedicalBag) {
    if(find(begin(props), end(props), propPoisoned) == end(props)) {

      const bool IS_RAPID_REC   = PlayerBon::hasTrait(Trait::rapidRecoverer);
      const bool IS_SURVIVALIST = PlayerBon::hasTrait(Trait::survivalist);

      const int REGEN_N_TURNS   = (IS_SURVIVALIST ? 20 : (IS_RAPID_REC ? 30 : 40));

      if(TURN % REGEN_N_TURNS == 0 && TURN > 1 && getHp() < getHpMax(true)) {++hp_;}
    }

    if(
      propHandler_->allowSee() &&
      find(begin(props), end(props), propConfused) == end(props)) {

      const int R = PlayerBon::hasTrait(Trait::perceptive) ? 3 :
                    (PlayerBon::hasTrait(Trait::observant) ? 2 : 1);

      int x0 = max(0, pos.x - R);
      int y0 = max(0, pos.y - R);
      int x1 = min(MAP_W - 1, pos.x + R);
      int y1 = min(MAP_H - 1, pos.y + R);

      for(int y = y0; y <= y1; ++y) {
        for(int x = x0; x <= x1; ++x) {
          if(Map::cells[x][y].isSeenByPlayer) {
            auto* f = Map::cells[x][y].rigid;

            if(f->getId() == FeatureId::trap) {
              static_cast<Trap*>(f)->playerTrySpotHidden();
            }
            if(f->getId() == FeatureId::door) {
              static_cast<Door*>(f)->playerTrySpotHidden();
            }
          }
        }
      }
    }
  }

  if(activeMedicalBag) {
    activeMedicalBag->continueAction();
  }

  if(waitTurnsLeft > 0) {
    waitTurnsLeft--;
    GameTime::actorDidAct();
  }
}

void Player::interruptActions() {
  Render::drawMapAndInterface();

  InvHandling::screenToOpenAfterDrop    = InvScrId::END;
  InvHandling::browserIdxToSetAfterDrop = 0;

  //Abort searching
  if(waitTurnsLeft > 0) {
    Log::addMsg("I stop waiting.", clrWhite);
    Render::drawMapAndInterface();
  }
  waitTurnsLeft = -1;

  if(activeMedicalBag) {
    activeMedicalBag->interrupted();
    activeMedicalBag = nullptr;
  }
}

void Player::hearSound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                       const Dir dirToOrigin,
                       const int PERCENT_AUDIBLE_DISTANCE) {
  const SfxId sfx = snd.getSfx();

  const string& msg = snd.getMsg();
  const bool HAS_SND_MSG = !msg.empty() && msg != " ";

  if(HAS_SND_MSG) {Log::addMsg(msg, clrWhite);}

  //Play audio after message to ensure synch between audio and animation
  //If origin is hidden, we only play the sound if there is a message
  if(HAS_SND_MSG || IS_ORIGIN_SEEN_BY_PLAYER) {
    Audio::play(sfx, dirToOrigin, PERCENT_AUDIBLE_DISTANCE);
  }

  if(HAS_SND_MSG) {
    Actor* const actorWhoMadeSnd = snd.getActorWhoMadeSound();
    if(actorWhoMadeSnd && actorWhoMadeSnd != this) {
      static_cast<Monster*>(actorWhoMadeSnd)->playerBecomeAwareOfMe();
    }
  }
}

void Player::moveDir(Dir dir) {
  if(deadState == ActorDeadState::alive) {

    propHandler_->changeMoveDir(pos, dir);

    //Trap affects leaving?
    if(dir != Dir::center) {
      Feature* f = Map::cells[pos.x][pos.y].rigid;
      if(f->getId() == FeatureId::trap) {
        TRACE << "Standing on trap, check if affects move" << endl;
        dir = static_cast<Trap*>(f)->actorTryLeave(*this, dir);
      }
    }

    bool isFreeTurn = false;;

    const Pos dest(pos + DirUtils::getOffset(dir));

    if(dir != Dir::center) {
      //Attack?
      Actor* const actorAtDest = Utils::getFirstActorAtPos(dest);
      if(actorAtDest) {
        if(propHandler_->allowAttackMelee(true)) {
          bool hasMeleeWpn = false;
          Item* const item = inv_->getItemInSlot(SlotId::wielded);
          if(item) {
            Wpn* const wpn = static_cast<Wpn*>(item);
            if(wpn->getData().melee.isMeleeWpn) {
              if(Config::isRangedWpnMeleeePrompt() &&
                  isSeeingActor(*actorAtDest, nullptr)) {
                if(wpn->getData().ranged.isRangedWpn) {
                  const string wpnName = wpn->getName(ItemRefType::a);
                  Log::addMsg(
                    "Attack " + actorAtDest->getNameThe() +
                    " with " + wpnName + "? (y/n)", clrWhiteHigh);
                  Render::drawMapAndInterface();
                  if(Query::yesOrNo() == YesNoAnswer::no) {
                    Log::clearLog();
                    Render::drawMapAndInterface();
                    return;
                  }
                }
              }
              Attack::melee(*this, *wpn, *actorAtDest);
              target = actorAtDest;
              return;
            }
          }
          if(!hasMeleeWpn) {punchMonster(*actorAtDest);}
        }
        return;
      }

      //This point reached means no actor in the destination cell.

      //Blocking mobile or rigid?
      vector<PropId> props;
      getPropHandler().getAllActivePropIds(props);
      Cell& cell = Map::cells[dest.x][dest.y];
      bool isFeaturesAllowMove = cell.rigid->canMove(props);

      vector<Mob*> mobs;
      GameTime::getMobsAtPos(dest, mobs);

      if(isFeaturesAllowMove) {
        for(auto* m : mobs) {
          if(!m->canMove(props)) {
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
          Render::drawMapAndInterface();
          return;
        } else if(ENC >= 100) {
          Log::addMsg("I stagger.", clrMsgWarning);
          propHandler_->tryApplyProp(new PropWaiting(PropTurns::std));
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
        if(item) {
          const bool CAN_SEE = propHandler_->allowSee();
          Log::addMsg(CAN_SEE ? "I see here:" : "I try to feel what is lying here...");
          Log::addMsg(item->getName(ItemRefType::plural, ItemRefInf::yes,
                                    ItemRefAttInf::wpnContext) + ".");
        }
      }

      //Note: bump() prints block messages.
      for(auto* m : mobs) {m->bump(*this);}

      Map::cells[dest.x][dest.y].rigid->bump(*this);
    }

    if(pos == dest) {
      GameTime::actorDidAct(isFreeTurn);
      return;
    }
  }
}

void Player::autoMelee() {
  if(target) {
    if(Utils::isPosAdj(pos, target->pos, false)) {
      if(isSeeingActor(*target, nullptr)) {
        moveDir(DirUtils::getDir(target->pos - pos));
        return;
      }
    }
  }

  //If this line reached, there is no adjacent cur target.
  for(int dx = -1; dx <= 1; ++dx) {
    for(int dy = -1; dy <= 1; ++dy) {
      if(dx != 0 || dy != 0) {
        Actor* const actor = Utils::getFirstActorAtPos(pos + Pos(dx, dy));
        if(actor) {
          if(isSeeingActor(*actor, nullptr)) {
            target = actor;
            moveDir(DirUtils::getDir(Pos(dx, dy)));
            return;
          }
        }
      }
    }
  }
}

void Player::kickMonster(Actor& actorToKick) {
  Wpn* kickWpn = nullptr;

  const ActorDataT& d = actorToKick.getData();

  if(d.actorSize == actorSize_floor && (d.isSpider || d.isRat)) {
    kickWpn = static_cast<Wpn*>(ItemFactory::mk(ItemId::playerStomp));
  } else {
    kickWpn = static_cast<Wpn*>(ItemFactory::mk(ItemId::playerKick));
  }
  Attack::melee(*this, *kickWpn, actorToKick);
  delete kickWpn;
}

void Player::punchMonster(Actor& actorToPunch) {
  //Spawn a temporary punch weapon to attack with
  Wpn* punchWpn = static_cast<Wpn*>(ItemFactory::mk(ItemId::playerPunch));
  Attack::melee(*this, *punchWpn, actorToPunch);
  delete punchWpn;
}

void Player::addLight_(bool light[MAP_W][MAP_H]) const {
  bool isUsingLightGivingItemSmall  = false; //3x3 cells
  bool isUsingLightGivingItemNormal = false;
  if(activeExplosive) {
    if(activeExplosive->getData().id == ItemId::flare) {
      isUsingLightGivingItemNormal = true;
    }
  }

  for(Item* const item : inv_->general_) {
    if(item->getData().id == ItemId::electricLantern) {
      DeviceLantern* const lantern = static_cast<DeviceLantern*>(item);
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
    Pos p0(max(0, pos.x - RADI), max(0, pos.y - RADI));
    Pos p1(min(MAP_W - 1, pos.x + RADI), min(MAP_H - 1, pos.y + RADI));

    bool visionBlockers[MAP_W][MAP_H];
    for(int y = p0.y; y <= p1.y; ++y) {
      for(int x = p0.x; x <= p1.x; ++x) {
        const auto* const f = Map::cells[x][y].rigid;
        visionBlockers[x][y] = !f->isVisionPassable();
      }
    }

    Fov::runFovOnArray(visionBlockers, pos, myLight, false);
    for(int y = p0.y; y <= p1.y; ++y) {
      for(int x = p0.x; x <= p1.x; ++x) {
        if(myLight[x][y]) {
          light[x][y] = true;
        }
      }
    }
  } else if(isUsingLightGivingItemSmall) {
    for(int y = pos.y - 1; y <= pos.y + 1; ++y) {
      for(int x = pos.x - 1; x <= pos.x + 1; ++x) {
        light[x][y] = true;
      }
    }
  }
}

void Player::updateFov() {
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      Map::cells[x][y].isSeenByPlayer = false;
    }
  }

  if(propHandler_->allowSee()) {
    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blocked);
    Fov::runPlayerFov(blocked, pos);
    Map::cells[pos.x][pos.y].isSeenByPlayer = true;
  }

  if(propHandler_->allowSee()) {FOVhack();}

  if(Init::isCheatVisionEnabled) {
    for(int y = 0; y < MAP_H; ++y) {
      for(int x = 0; x < MAP_W; ++x) {
        Map::cells[x][y].isSeenByPlayer = true;
      }
    }
  }

  //Explore
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      Cell& cell = Map::cells[x][y];
      const bool IS_BLOCKING = CellPred::BlocksMoveCmn(false).check(cell);
      //Do not explore dark floor cells
      if(cell.isSeenByPlayer && (!cell.isDark || IS_BLOCKING)) {
        cell.isExplored = true;
      }
    }
  }
}

void Player::FOVhack() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      if(visionBlockers[x][y] && blocked[x][y]) {
        for(int dy = -1; dy <= 1; ++dy) {
          for(int dx = -1; dx <= 1; ++dx) {
            const Pos adj(x + dx, y + dy);
            if(Utils::isPosInsideMap(adj)) {
              const Cell& adjCell = Map::cells[adj.x][adj.y];
              if(
                adjCell.isSeenByPlayer &&
                (!adjCell.isDark || adjCell.isLight) &&
                !blocked[adj.x][adj.y]) {
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
