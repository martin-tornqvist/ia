#include "FeatureExaminable.h"

#include <vector>
#include <assert.h>

#include "Init.h"
#include "Log.h"
#include "Renderer.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemFactory.h"
#include "ItemDrop.h"
#include "Popup.h"
#include "PlayerBon.h"
#include "ActorPlayer.h"
#include "Inventory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Explosion.h"
#include "PopulateMonsters.h"
#include "Map.h"
#include "Utils.h"

using namespace std;

//---------------------------------------------------------ITEM CONTAINE
ItemContainerFeature::ItemContainerFeature() {items_.resize(0);}

ItemContainerFeature::~ItemContainerFeature() {
  for(unsigned int i = 0; i < items_.size(); ++i) {
    delete items_.at(i);
  }
}

void ItemContainerFeature::setRandomItemsForFeature(
  const FeatureId featureId,
  const int NR_ITEMS_TO_ATTEMPT) {
  for(unsigned int i = 0; i < items_.size(); ++i) {
    delete items_.at(i);
  }
  items_.resize(0);

  if(NR_ITEMS_TO_ATTEMPT > 0) {
    while(items_.empty()) {
      vector<ItemId> itemBucket;
      for(int i = 1; i < int(ItemId::END); ++i) {
        ItemDataT* const curData = ItemData::data[i];
        for(
          unsigned int ii = 0;
          ii < curData->featuresCanBeFoundIn.size();
          ii++) {
          auto featuresFoundIn = curData->featuresCanBeFoundIn.at(ii);
          if(featuresFoundIn.first == featureId) {
            if(Rnd::percentile() < featuresFoundIn.second) {
              if(Rnd::percentile() < curData->chanceToIncludeInSpawnList) {
                itemBucket.push_back(ItemId(i));
                break;
              }
            }
          }
        }
      }

      const int NR_CANDIDATES = int(itemBucket.size());
      if(NR_CANDIDATES > 0) {
        for(int i = 0; i < NR_ITEMS_TO_ATTEMPT; ++i) {
          const auto ELEMENT = Rnd::range(0, NR_CANDIDATES - 1);
          Item* item = ItemFactory::mk(itemBucket.at(ELEMENT));
          ItemFactory::setItemRandomizedProperties(item);
          items_.push_back(item);
        }
      }
    }
  }
}

void ItemContainerFeature::dropItems(const Pos& pos) {
  for(auto* item : items_) {ItemDrop::dropItemOnMap(pos, *item);}
  items_.resize(0);
}

void ItemContainerFeature::destroySingleFragile() {
  //TODO Generalize this function (perhaps isFragile variable in item data)

  for(size_t i = 0; i < items_.size(); ++i) {
    Item* const item = items_.at(i);
    const ItemDataT& d = item->getData();
    if(d.isPotion || d.id == ItemId::molotov) {
      delete item;
      items_.erase(items_.begin() + i);
      Log::addMsg("I hear a muffled shatter.");
      break;
    }
  }
}

//--------------------------------------------------------- TOMB
Tomb::Tomb(const Pos& pos) :
  FeatureStatic(pos), isContentKnown_(false), isTraitKnown_(false),
  pushLidOneInN_(Rnd::range(6, 14)), appearance_(TombAppearance::common),
  trait_(TombTrait::END) {

  //Contained items
  const int NR_ITEMS_MIN = Rnd::oneIn(3) ? 0 : 1;
  const int NR_ITEMS_MAX =
    NR_ITEMS_MIN + (PlayerBon::hasTrait(Trait::treasureHunter) ? 1 : 0);

  itemContainer_.setRandomItemsForFeature(
    FeatureId::tomb, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

  //Appearance
  if(Rnd::oneIn(5)) {
    const TombAppearance lastAppearance = TombAppearance::END;
    appearance_ = TombAppearance(Rnd::range(0, int(lastAppearance) - 1));
  } else {
    for(Item* item : itemContainer_.items_) {
      const ItemValue itemValue = item->getData().itemValue;
      if(itemValue == ItemValue::majorTreasure) {
        appearance_ = TombAppearance::marvelous;
        break;
      } else if(itemValue == ItemValue::minorTreasure) {
        appearance_ = TombAppearance::ornate;
      }
    }
  }

  if(!itemContainer_.items_.empty()) {
    const int RND = Rnd::percentile();
    if(RND < 15) {
      trait_ = TombTrait::forebodingCarvedSigns;
    } else if(RND < 45) {
      trait_ = TombTrait::stench;
    } else if(RND < 75) {
      trait_ = TombTrait::auraOfUnrest;
    }
  }
}

string Tomb::getDescr(const bool DEFINITE_ARTICLE) const {
  switch(appearance_) {
    case TombAppearance::common:
      return string(DEFINITE_ARTICLE ? "the" : "a")  + " tomb";
    case TombAppearance::ornate:
      return string(DEFINITE_ARTICLE ? "the" : "an") + " ornate tomb";
    case TombAppearance::marvelous:
      return string(DEFINITE_ARTICLE ? "the" : "a")  + " marvelous tomb";
    case TombAppearance::END: {} break;
  }
  assert("Failed to set Tomb description" && false);
  return "";
}

Clr Tomb::getClr() const {
  switch(appearance_) {
    case TombAppearance::common:    return clrGray;
    case TombAppearance::ornate:    return clrWhite;
    case TombAppearance::marvelous: return clrYellow;
    case TombAppearance::END: {} break;
  }
  assert("Failed to set Tomb color" && false);
  return clrBlack;
}

void Tomb::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      Log::addMsg("The tomb is empty.");
    } else {
      Log::addMsg("I attempt to push the lid.");

      vector<PropId> props;
      Map::player->getPropHandler().getAllActivePropIds(props);

      if(find(begin(props), end(props), propWeakened) != end(props)) {
        trySprainPlayer();
        Log::addMsg("It seems futile.");
      } else {
        const int BON = PlayerBon::hasTrait(Trait::rugged) ? 8 :
                        PlayerBon::hasTrait(Trait::tough)  ? 4 : 0;

        TRACE << "Tomb: Base chance to push lid is: 1 in ";
        TRACE << pushLidOneInN_ << endl;

        TRACE << "Tomb: Bonus to roll: " << BON << endl;

        const int ROLL_TOT = Rnd::range(1, pushLidOneInN_) + BON;

        TRACE << "Tomb: Roll + bonus = " << ROLL_TOT << endl;

        bool isSuccess = false;

        if(ROLL_TOT < pushLidOneInN_ - 9) {
          Log::addMsg("It does not yield at all.");
        } else if(ROLL_TOT < pushLidOneInN_ - 1) {
          Log::addMsg("It resists.");
        } else if(ROLL_TOT == pushLidOneInN_ - 1) {
          Log::addMsg("It moves a little!");
          pushLidOneInN_--;
        } else {
          isSuccess = true;
        }

        if(isSuccess) {open();} else  {trySprainPlayer();}
      }
      GameTime::actorDidAct();
    }
  }
}

void Tomb::trySprainPlayer() {
  const int SPRAIN_ONE_IN_N = PlayerBon::hasTrait(Trait::rugged) ? 6 :
                              PlayerBon::hasTrait(Trait::tough)  ? 5 : 4;
  if(Rnd::oneIn(SPRAIN_ONE_IN_N)) {
    Log::addMsg("I sprain myself.", clrMsgBad);
    Map::player->hit(Rnd::range(1, 5), DmgType::pure, false);
  }
}

bool Tomb::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  if(IS_SEEN) {
    Log::addMsg("The lid comes off!");
    Log::addMsg("The tomb opens.");
  }
  Snd snd("I hear heavy stone sliding.", SfxId::tombOpen,
          IgnoreMsgIfOriginSeen::yes, pos_, nullptr, SndVol::high,
          AlertsMonsters::yes);
  SndEmit::emitSnd(snd);

  triggerTrap(*Map::player);
  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) Log::addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_);
  } else {
    if(IS_SEEN)Log::addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) {isContentKnown_ = isTraitKnown_ = true;}
  Renderer::drawMapAndInterface();
  return true;
}

void Tomb::examine() {
  vector<PropId> props;
  Map::player->getPropHandler().getAllActivePropIds(props);

  if(find(begin(props), end(props), propConfused) != end(props)) {
    Log::addMsg("I start to search the tomb...");
    Log::addMsg("but I cannot grasp what for.");
    GameTime::actorDidAct();
  } else if(itemContainer_.items_.empty() && isContentKnown_) {
    Log::addMsg("The tomb is empty.");
  } else {
    if(!isTraitKnown_ && trait_ != TombTrait::END) {
      const int FIND_ONE_IN_N = PlayerBon::hasTrait(Trait::perceptive) ? 2 :
                                (PlayerBon::hasTrait(Trait::observant) ? 3 : 6);

      isTraitKnown_ = Rnd::oneIn(FIND_ONE_IN_N);
    }

    if(isTraitKnown_) {
      switch(trait_) {
        case TombTrait::auraOfUnrest: {
          Log::addMsg("It has a certain aura of unrest about it.");
        } break;

        case TombTrait::forebodingCarvedSigns: {
          if(PlayerBon::getBg() == Bg::occultist) {
            Log::addMsg("There is a curse carved on the box.");
          } else {
            Log::addMsg("There are some ominous runes carved on the box.");
          }
        } break;

        case TombTrait::stench: {
          Log::addMsg("There is a pungent stench.");
        } break;

        case TombTrait::END: {} break;
      }
    } else {
      Log::addMsg("I find nothing significant.");
    }
    GameTime::actorDidAct();
  }
}

//void Tomb::kick(Actor& actorTrying) {
//  (void)actorTrying;

//  const Inventory& inv = Map::player->getInv();
//  bool hasSledgehammer = false;
//  Item* item = inv.getItemInSlot(SlotId::wielded);
//  if(item) {
//    hasSledgehammer = item->getData().id == ItemId::sledgeHammer;
//  }
//  if(!hasSledgehammer) {
//    item = inv.getItemInSlot(SlotId::wieldedAlt);
//    hasSledgehammer = item->getData().id == ItemId::sledgeHammer;
//  }
//  if(!hasSledgehammer) {
//    hasSledgehammer = inv.hasItemInGeneral(ItemId::sledgeHammer);
//  }
//  if(hasSledgehammer) {
//    possibleActions.push_back(tombAction_smashLidWithSledgehammer);
//  }

//  Log::addMsg("I strike at the lid with a Sledgehammer.");
//  const int BREAK_N_IN_10 = IS_WEAK ? 1 : 8;
//  if(Rnd::fraction(BREAK_N_IN_10, 10)) {
//    Log::addMsg("The lid cracks open!");
//    if(!IS_BLESSED && (IS_CURSED || Rnd::oneIn(3))) {
//      itemContainer_.destroySingleFragile();
//    }
//    open();
//  } else {
//    Log::addMsg("The lid resists.");
//  }

//  GameTime::actorDidAct();
//}

//void Tomb::disarm() {

//case tombAction_carveCurseWard: {
//    if(!IS_CURSED && (IS_BLESSED || Rnd::fraction(4, 5))) {
//      Log::addMsg("The curse is cleared.");
//    } else {
//      Log::addMsg("I make a mistake, the curse is doubled!");
//      PropCursed* const curse =
//        new PropCursed(propTurnsStd);
//      curse->turnsLeft_ *= 2;
//      propHlr.tryApplyProp(curse, true);
//    }
//    trait_ = endOfTombTraits;
//  } break;
//}

void Tomb::triggerTrap(Actor& actor) {
  (void)actor;

  vector<ActorId> actorBucket;

  switch(trait_) {
    case TombTrait::auraOfUnrest: {

      for(int i = 1; i < endOfActorIds; ++i) {
        const ActorDataT& d = ActorData::data[i];
        if(
          d.isGhost && d.isAutoSpawnAllowed && !d.isUnique &&
          ((Map::dlvl + 5) >= d.spawnMinDLVL ||
           Map::dlvl >= MIN_DLVL_HARDER_TRAPS)) {
          actorBucket.push_back(ActorId(i));
        }
      }
      Log::addMsg("Something rises from the tomb!");
    } break;

    case TombTrait::forebodingCarvedSigns: {
      Map::player->getPropHandler().tryApplyProp(
        new PropCursed(propTurnsStd));
    } break;

    case TombTrait::stench: {
      if(Rnd::coinToss()) {
        Log::addMsg("Fumes burst out from the tomb!");
        Prop* prop = nullptr;
        Clr fumeClr = clrMagenta;
        const int RND = Rnd::percentile();
        if(RND < 20) {
          prop = new PropPoisoned(propTurnsStd);
          fumeClr = clrGreenLgt;
        } else if(RND < 40) {
          prop = new PropDiseased(propTurnsSpecific, 50);
          fumeClr = clrGreen;
        } else {
          prop = new PropParalyzed(propTurnsStd);
          prop->turnsLeft_ *= 2;
        }
        Explosion::runExplosionAt(
          pos_, ExplType::applyProp, ExplSrc::misc, 0, SfxId::END,
          prop, &fumeClr);
      } else {
        for(int i = 1; i < endOfActorIds; ++i) {
          const ActorDataT& d = ActorData::data[i];
          if(
            d.intrProps[propOoze] &&
            d.isAutoSpawnAllowed  &&
            !d.isUnique) {
            actorBucket.push_back(ActorId(i));
          }
        }
        Log::addMsg("Something creeps up from the tomb!");
      }

    } break;

    default: {} break;
  }

  if(!actorBucket.empty()) {
    const unsigned int ELEM = Rnd::range(0, actorBucket.size() - 1);
    const ActorId actorIdToSpawn = actorBucket.at(ELEM);
    Actor* const monster = ActorFactory::mk(actorIdToSpawn, pos_);
    static_cast<Monster*>(monster)->becomeAware(false);
  }
}

//--------------------------------------------------------- CHEST
Chest::Chest(const Pos& pos) :
  FeatureStatic(pos),
  isContentKnown_(false),
  isLocked_(false),
  isTrapped_(false),
  isTrapStatusKnown_(false),
  matl(ChestMatl(Rnd::range(0, int(ChestMatl::END) - 1))) {

  const bool IS_TREASURE_HUNTER =
    PlayerBon::hasTrait(Trait::treasureHunter);
  const int NR_ITEMS_MIN = Rnd::oneIn(10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 3 : 2;
  itemContainer_.setRandomItemsForFeature(
    FeatureId::chest, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

  if(!itemContainer_.items_.empty()) {
    isLocked_   = Rnd::fraction(6, 10);
    isTrapped_  = Rnd::fraction(6, 10);
  }
}

void Chest::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      Log::addMsg("The chest is empty.");
    } else {
      if(isLocked_) {Log::addMsg("The chest is locked.");} else {open();}
      GameTime::actorDidAct();
    }
  }
}

void Chest::trySprainPlayer() {
  const int SPRAIN_ONE_IN_N = PlayerBon::hasTrait(Trait::rugged) ? 6 :
                              PlayerBon::hasTrait(Trait::tough)  ? 5 : 4;
  if(Rnd::oneIn(SPRAIN_ONE_IN_N)) {
    Log::addMsg("I sprain myself.", clrMsgBad);
    Map::player->hit(Rnd::range(1, 5), DmgType::pure, false);
  }
}

bool Chest::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;

  if(IS_SEEN) Log::addMsg("The chest opens.");
  triggerTrap(*Map::player);

  if(Map::player->deadState == ActorDeadState::alive) {
    if(itemContainer_.items_.empty()) {
      if(IS_SEEN) Log::addMsg("There is nothing of value inside.");
    } else {
      if(IS_SEEN) Log::addMsg("There are some items inside.");
      itemContainer_.dropItems(pos_);
    }
  }
  if(IS_SEEN) {
    isContentKnown_     = true;
    isTrapStatusKnown_  = true;
  }
  isLocked_ = false;
  Renderer::drawMapAndInterface();
  return true;
}

//void Chest::hit_(const DmgType dmgType, const DmgMethod dmgMethod,
//                 Actor* const actor) {
//  (void)actor;
//
//  switch(dmgType) {
//    case DmgType::physical: {
//      switch(dmgMethod) {
//        case DmgMethod::kick: {
//          if(itemContainer_.items_.empty() && isContentKnown_) {
//            Log::addMsg("The chest is empty.");
//          } else {
//
//            Log::addMsg("I kick the lid.");
//
//            vector<PropId> props;
//            Map::player->getPropHandler().getAllActivePropIds(props);
//
//            if(
//              find(begin(props), end(props), propWeakened) != end(props) ||
//              matl == ChestMatl::iron) {
//              trySprainPlayer();
//              Log::addMsg("It seems futile.");
//            } else {
//
//              const bool IS_CURSED
//                = find(begin(props), end(props), propCursed)  != end(props);
//              const bool IS_BLESSED
//                = find(begin(props), end(props), propBlessed) != end(props);
//
//              if(!IS_BLESSED && (IS_CURSED || Rnd::oneIn(3))) {
//                itemContainer_.destroySingleFragile();
//              }
//
//              const bool IS_TOUGH   = PlayerBon::hasTrait(Trait::tough);
//              const bool IS_RUGGED  = PlayerBon::hasTrait(Trait::rugged);
//
//              const int OPEN_ONE_IN_N = IS_RUGGED ? 2 : IS_TOUGH ? 3 : 5;
//
//              if(Rnd::oneIn(OPEN_ONE_IN_N)) {
//                Log::addMsg("I kick the lid open!");
//                open();
//              } else {
//                Log::addMsg("The lock resists.");
//                trySprainPlayer();
//              }
//            }
//            GameTime::actorDidAct();
//          }
//        } break;
//
//        default: {} break;
//
//      } //dmgMethod
//
//    } break;
//
//    default: {} break;
//
//  } //dmgType
//
//  //TODO Force lock with weapon
////      Inventory& inv    = Map::player->getInv();
////      Item* const item  = inv.getItemInSlot(SlotId::wielded);
////
////      if(!item) {
////        Log::addMsg(
////          "I attempt to punch the lock open, nearly breaking my hand.",
////          clrMsgBad);
////        Map::player->hit(1, DmgType::pure, false);
////      } else {
////        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);
////
////        if(Rnd::percentile() < CHANCE_TO_DMG_WPN) {
////          const string wpnName = ItemData::getItemRef(
////                                   *item, ItemRefType::plain, true);
////
////          Weapon* const wpn = static_cast<Weapon*>(item);
////
////          if(wpn->meleeDmgPlus == 0) {
////            Log::addMsg("My " + wpnName + " breaks!");
////            delete wpn;
////            inv.getSlot(SlotId::wielded)->item = nullptr;
////          } else {
////            Log::addMsg("My " + wpnName + " is damaged!");
////            wpn->meleeDmgPlus--;
////          }
////          return;
////        }
////
////        if(IS_WEAK) {
////          Log::addMsg("It seems futile.");
////        } else {
////          const int CHANCE_TO_OPEN = 40;
////          if(Rnd::percentile() < CHANCE_TO_OPEN) {
////            Log::addMsg("I force the lock open!");
////            open();
////          } else {
////            Log::addMsg("The lock resists.");
////          }
////        }
////      }
//}

void Chest::examine() {
  vector<PropId> props;
  Map::player->getPropHandler().getAllActivePropIds(props);

  if(find(begin(props), end(props), propConfused) != end(props)) {
    Log::addMsg("I start to search the chest...");
    Log::addMsg("but I cannot grasp the purpose.");
    GameTime::actorDidAct();
  } else if(itemContainer_.items_.empty() && isContentKnown_) {
    Log::addMsg("The chest is empty.");
  } else {
    if(isLocked_) {
      Log::addMsg("The chest is locked.");
    }

    const int FIND_ONE_IN_N = PlayerBon::hasTrait(Trait::perceptive) ? 3 :
                              (PlayerBon::hasTrait(Trait::observant) ? 4 : 7);

    if(isTrapped_ && (isTrapStatusKnown_ || (Rnd::oneIn(FIND_ONE_IN_N)))) {
      Log::addMsg("There appears to be a hidden trap mechanism!");
      isTrapStatusKnown_ = true;
    } else {
      Log::addMsg("I find nothing unusual about it.");
    }
    GameTime::actorDidAct();
  }
}

void Chest::disarm() {
  //Try disarming trap
  if(isTrapped_ && isTrapStatusKnown_) {
    Log::addMsg("I attempt to disarm the trap.");

    const int TRIGGER_ONE_IN_N = 5;
    if(Rnd::oneIn(TRIGGER_ONE_IN_N)) {
      Log::addMsg("I set off the trap!");
      triggerTrap(*Map::player);
    } else {

      const int DISARM_ONE_IN_N = 2;

      if(Rnd::oneIn(DISARM_ONE_IN_N)) {
        Log::addMsg("I successfully disarm it!");
        isTrapped_ = false;
      } else {
        Log::addMsg("I failed to disarm it.");
      }
    }
    GameTime::actorDidAct();
    return;
  }


  //Try picking the lock //TODO Implement
//  if(isLocked_) {
//      GameTime::actorDidAct();
//  }

  //If no other action was taken, try examining the chest instead
  examine();
}

void Chest::triggerTrap(Actor& actor) {
  (void)actor;

  isTrapStatusKnown_ = true;

  if(isTrapped_) {

    isTrapped_ = false;

    const int EXPLODE_ONE_IN_N = 7;
    if(
      Map::dlvl >= MIN_DLVL_HARDER_TRAPS &&
      Rnd::oneIn(EXPLODE_ONE_IN_N)) {
      Log::addMsg("The trap explodes!");
      Explosion::runExplosionAt(pos_, ExplType::expl, ExplSrc::misc, 0,
                                SfxId::explosion);
      if(Map::player->deadState == ActorDeadState::alive) {
        Map::put(new RubbleLow(pos_));
      }
    } else {
      Log::addMsg("Fumes burst out from the chest!");
      Prop* prop = nullptr;
      Clr fumeClr = clrMagenta;
      const int RND = Rnd::percentile();
      if(RND < 20) {
        prop = new PropPoisoned(propTurnsStd);
        fumeClr = clrGreenLgt;
      } else if(RND < 40) {
        prop = new PropDiseased(propTurnsStd);
        fumeClr = clrGreen;
      } else {
        prop = new PropParalyzed(propTurnsStd);
        prop->turnsLeft_ *= 2;
      }
      Explosion::runExplosionAt(pos_, ExplType::applyProp, ExplSrc::misc,
                                0, SfxId::END, prop, &fumeClr);
    }
  }
}

//--------------------------------------------------------- FOUNTAIN
Fountain::Fountain(const Pos& pos) :
  FeatureStatic(pos), fountainType_(FountainType::tepid),
  fountainMatl_(FountainMatl::stone) {

  if(Rnd::oneIn(4)) {fountainMatl_ = FountainMatl::gold;}

  switch(fountainMatl_) {
    case FountainMatl::stone: {
      const int NR_TYPES = int(FountainType::END);
      fountainType_ = FountainType(Rnd::range(1, NR_TYPES - 1));
    } break;

    case FountainMatl::gold: {
      vector<FountainType> typeBucket {
        FountainType::bless, FountainType::refreshing, FountainType::spirit,
        FountainType::vitality, FountainType::rFire, FountainType::rCold,
        FountainType::rElec, FountainType::rFear, FountainType::rConfusion
      };
      const int ELEMENT = Rnd::range(0, typeBucket.size() - 1);
      fountainType_ = typeBucket.at(ELEMENT);
    } break;
  }
}

Clr Fountain::getClr() const {
  switch(fountainMatl_) {
    case FountainMatl::stone: return clrGray;
    case FountainMatl::gold:  return clrYellow;
  }
  assert("Failed to get fountain color" && false);
  return clrBlack;
}

string Fountain::getDescr(const bool DEFINITE_ARTICLE) const {
  string article = DEFINITE_ARTICLE ? "the" : "a";

  switch(fountainMatl_) {
    case FountainMatl::stone: return article + " stone fountain";
    case FountainMatl::gold:  return article + " golden fountain";
  }
  assert("Failed to get fountain description" && false);
  return "";
}

void Fountain::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {

    if(fountainType_ == FountainType::dry) {
      Log::addMsg("The fountain is dried out.");
    } else {
      PropHandler& propHlr = Map::player->getPropHandler();

      Log::addMsg("I drink from the fountain...");

      switch(fountainType_) {
        case FountainType::dry: {} break;

        case FountainType::tepid: {
          Log::addMsg("The water is tepid.");
        } break;

        case FountainType::refreshing: {
          Log::addMsg("It's very refreshing.");
          Map::player->restoreHp(1, false);
          Map::player->restoreSpi(1, false);
          Map::player->restoreShock(5, false);
        } break;

        case FountainType::bless: {
          propHlr.tryApplyProp(new PropBlessed(propTurnsStd));
        } break;

        case FountainType::curse: {
          propHlr.tryApplyProp(new PropCursed(propTurnsStd));
        } break;

        case FountainType::spirit: {
          Map::player->restoreSpi(2, true, true);
        } break;

        case FountainType::vitality: {
          Map::player->restoreHp(2, true, true);
        } break;

        case FountainType::disease: {
          propHlr.tryApplyProp(new PropDiseased(propTurnsSpecific, 50));
        } break;

        case FountainType::poison: {
          propHlr.tryApplyProp(new PropPoisoned(propTurnsStd));
        } break;

        case FountainType::frenzy: {
          propHlr.tryApplyProp(new PropFrenzied(propTurnsStd));
        } break;

        case FountainType::paralyze: {
          propHlr.tryApplyProp(new PropParalyzed(propTurnsStd));
        } break;

        case FountainType::blind: {
          propHlr.tryApplyProp(new PropBlind(propTurnsStd));
        } break;

        case FountainType::faint: {
          propHlr.tryApplyProp(new PropFainted(propTurnsSpecific, 10));
        } break;

        case FountainType::rFire: {
          propHlr.tryApplyProp(new PropRFire(propTurnsStd));
        } break;

        case FountainType::rCold: {
          propHlr.tryApplyProp(new PropRCold(propTurnsStd));
        } break;

        case FountainType::rElec: {
          propHlr.tryApplyProp(new PropRElec(propTurnsStd));
        } break;

        case FountainType::rConfusion: {
          propHlr.tryApplyProp(new PropRConfusion(propTurnsStd));
        } break;

        case FountainType::rFear: {
          propHlr.tryApplyProp(new PropRFear(propTurnsStd));
        } break;

        case FountainType::END: {} break;
      }

      if(Rnd::oneIn(5)) {
        Log::addMsg("The fountain dries out.");
        fountainType_ = FountainType::dry;
      }
    }
  }
  GameTime::actorDidAct();
}


//--------------------------------------------------------- CABINET
Cabinet::Cabinet(const Pos& pos) : FeatureStatic(pos), isContentKnown_(false) {
  const int IS_EMPTY_N_IN_10  = 5;
  const int NR_ITEMS_MIN      = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX      = PlayerBon::hasTrait(Trait::treasureHunter) ? 2 : 1;
  itemContainer_.setRandomItemsForFeature(
    FeatureId::cabinet, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
}

void Cabinet::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {

    if(itemContainer_.items_.empty() && isContentKnown_) {
      Log::addMsg("The cabinet is empty.");
    } else {
      open();
    }
  }
}

bool Cabinet::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;

  if(IS_SEEN) Log::addMsg("The cabinet opens.");

  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) Log::addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_);
  } else {
    if(IS_SEEN) Log::addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) isContentKnown_ = true;
  Renderer::drawMapAndInterface(true);
  return true;
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(const Pos& pos) : FeatureStatic(pos), isContentKnown_(false) {
  const bool IS_TREASURE_HUNTER = PlayerBon::hasTrait(Trait::treasureHunter);
  const int IS_EMPTY_N_IN_10    = 6;
  const int NR_ITEMS_MIN        = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX        = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);
  itemContainer_.setRandomItemsForFeature(
    FeatureId::cocoon, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
}

void Cocoon::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      Log::addMsg("The cocoon is empty.");
    } else {
      open();
    }
  }
}

void Cocoon::triggerTrap(Actor& actor) {
  (void)actor;

  const int RND = Rnd::percentile();

  if(RND < 15) {
    Log::addMsg("There is a half-dissolved human body inside!");
    Map::player->incrShock(ShockValue::shockValue_heavy, ShockSrc::misc);
  } else if(RND < 50) {
    TRACE << "Cocoon: Attempting to spawn spiders" << endl;
    vector<ActorId> spawnBucket;
    for(int i = 1; i < endOfActorIds; ++i) {
      const ActorDataT& d = ActorData::data[i];
      if(
        d.isSpider && d.actorSize == actorSize_floor &&
        !d.isAutoSpawnAllowed && d.isUnique) {
        spawnBucket.push_back(d.id);
      }
    }

    const int NR_CANDIDATES = spawnBucket.size();
    if(NR_CANDIDATES > 0) {
      TRACE << "Cocoon: Spawn candidates found, attempting to place" << endl;
      Log::addMsg("There are spiders inside!");
      const int NR_SPIDERS = Rnd::range(2, 5);
      const int ELEMENT = Rnd::range(0, NR_CANDIDATES - 1);
      const ActorId actorIdToSummon = spawnBucket.at(ELEMENT);
      ActorFactory::summonMonsters(
        pos_, vector<ActorId>(NR_SPIDERS, actorIdToSummon), true);
    }
  }
}

bool Cocoon::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  if(IS_SEEN) Log::addMsg("The cocoon opens.");
  triggerTrap(*Map::player);
  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) Log::addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_);
  } else {
    if(IS_SEEN) Log::addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) isContentKnown_ = true;
  Renderer::drawMapAndInterface(true);

  return true;
}

//--------------------------------------------------------- ALTAR
//Altar::Altar(const Pos& pos) :
//  FeatureStatic(id, pos) {}
//
//void Altar::featureSpecific_examine() {
//}

//--------------------------------------------------------- CARVED PILLAR
//CarvedPillar::CarvedPillar(const Pos& pos) :
//  FeatureStatic(id, pos) {}
//
//void CarvedPillar::featureSpecific_examine() {
//}

//--------------------------------------------------------- BARREL
//Barrel::Barrel(const Pos& pos) :
//  FeatureStatic(id, pos) {}
//
//void Barrel::featureSpecific_examine() {
//}
