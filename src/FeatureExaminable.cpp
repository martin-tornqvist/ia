#include "FeatureExaminable.h"

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
#include "FeatureFactory.h"
#include "Utils.h"

//---------------------------------------------------------ITEM CONTAINE
ItemContainerFeature::ItemContainerFeature() {items_.resize(0);}

ItemContainerFeature::~ItemContainerFeature() {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
}

void ItemContainerFeature::setRandomItemsForFeature(
  const FeatureId featureId,
  const int NR_ITEMS_TO_ATTEMPT) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
  items_.resize(0);

  if(NR_ITEMS_TO_ATTEMPT > 0) {
    while(items_.empty()) {
      vector<ItemId> itemCandidates;
      for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
        ItemData* const curData = engine.itemDataHandler->dataList[i];
        for(
          unsigned int ii = 0;
          ii < curData->featuresCanBeFoundIn.size();
          ii++) {
          pair<FeatureId, int> featuresFoundIn =
            curData->featuresCanBeFoundIn.at(ii);
          if(featuresFoundIn.first == featureId) {
            if(Rnd::percentile() < featuresFoundIn.second) {
              if(Rnd::percentile() < curData->chanceToIncludeInSpawnList) {
                itemCandidates.push_back(ItemId(i));
                break;
              }
            }
          }
        }
      }

      const int NR_CANDIDATES = int(itemCandidates.size());
      if(NR_CANDIDATES > 0) {
        for(int i = 0; i < NR_ITEMS_TO_ATTEMPT; i++) {
          const unsigned int ELEMENT =
            Rnd::range(0, NR_CANDIDATES - 1);
          Item* item =
            engine.itemFactory->spawnItem(itemCandidates.at(ELEMENT));
          engine.itemFactory->setItemRandomizedProperties(item);
          items_.push_back(item);
        }
      }
    }
  }
}

void ItemContainerFeature::dropItems(const Pos& pos) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    engine.itemDrop->dropItemOnMap(pos, *items_.at(i));
  }
  items_.resize(0);
}

void ItemContainerFeature::destroySingleFragile() {
  //TODO Generalize this function (perhaps isFragile variable in item data)

  for(unsigned int i = 0; i < items_.size(); i++) {
    Item* const item = items_.at(i);
    const ItemData& d = item->getData();
    if(d.isPotion || d.id == ItemId::molotov) {
      delete item;
      items_.erase(items_.begin() + i);
      engine.log->addMsg("I hear a muffled shatter.");
      break;
    }
  }
}

//--------------------------------------------------------- TOMB
Tomb::Tomb(FeatureId id, Pos pos) :
  FeatureStatic(id, pos), isContentKnown_(false),
  isTraitKnown_(false), pushLidOneInN_(Rnd::range(6, 14)),
  appearance_(TombAppearance::common), trait_(TombTrait::endOfTombTraits) {

  //Contained items
  const int NR_ITEMS_MIN = Rnd::oneIn(3) ? 0 : 1;
  const int NR_ITEMS_MAX =
    NR_ITEMS_MIN + (PlayerBon::hasTrait(Trait::treasureHunter) ? 1 : 0);

  itemContainer_.setRandomItemsForFeature(
    feature_tomb, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  //Appearance
  if(Rnd::oneIn(5)) {
    const TombAppearance lastAppearance = TombAppearance::endOfTombAppearance;
    appearance_ = TombAppearance(Rnd::range(0, int(lastAppearance) - 1));
  } else {
    for(Item * item : itemContainer_.items_) {
      const ItemValue itemValue = item->getData().itemValue;
      if(itemValue == ItemValue::majorTreasure) {
        appearance_ = TombAppearance::marvelous;
        break;
      } else if(itemValue == ItemValue::minorTreasure) {
        appearance_ = TombAppearance::ornate;
      }
    }
  }

  const bool IS_CONTAINING_ITEMS = itemContainer_.items_.empty() == false;

  if(IS_CONTAINING_ITEMS) {
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
    case TombAppearance::endOfTombAppearance: {} break;
  }
  assert("Failed to set Tomb description" && false);
  return "";
}

SDL_Color Tomb::getClr() const {
  switch(appearance_) {
    case TombAppearance::common:    return clrGray;
    case TombAppearance::ornate:    return clrWhite;
    case TombAppearance::marvelous: return clrYellow;
    case TombAppearance::endOfTombAppearance: {} break;
  }
  assert("Failed to set Tomb color" && false);
  return clrBlack;
}

void Tomb::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The tomb is empty.");
    } else {
      eng.log->addMsg("I attempt to push the lid.");

      vector<PropId> props;
      Map::player->getPropHandler().getAllActivePropIds(props);

      if(find(props.begin(), props.end(), propWeakened) != props.end()) {
        trySprainPlayer();
        eng.log->addMsg("It seems futile.");
      } else {
        const int BON = PlayerBon::hasTrait(Trait::rugged) ? 8 :
                        PlayerBon::hasTrait(Trait::tough)  ? 4 : 0;

        trace << "Tomb: Base chance to push lid is: 1 in ";
        trace << pushLidOneInN_ << endl;

        trace << "Tomb: Bonus to roll: " << BON << endl;

        const int ROLL_TOT = Rnd::range(1, pushLidOneInN_) + BON;

        trace << "Tomb: Roll + bonus = " << ROLL_TOT << endl;

        bool isSuccess = false;

        if(ROLL_TOT < pushLidOneInN_ - 9) {
          eng.log->addMsg("It does not yield at all.");
        } else if(ROLL_TOT < pushLidOneInN_ - 1) {
          eng.log->addMsg("It resists.");
        } else if(ROLL_TOT == pushLidOneInN_ - 1) {
          eng.log->addMsg("It moves a little!");
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
    eng.log->addMsg("I sprain myself.", clrMsgBad);
    Map::player->hit(Rnd::range(1, 5), DmgType::pure, false);
  }
}

bool Tomb::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  if(IS_SEEN) {
    eng.log->addMsg("The lid comes off!");
    eng.log->addMsg("The tomb opens.");
  }
  Snd snd("I hear heavy stone sliding.", SfxId::tombOpen,
          IgnoreMsgIfOriginSeen::yes, pos_, NULL, SndVol::high,
          AlertsMonsters::yes);
  SndEmit::emitSnd(snd, eng);

  triggerTrap(*Map::player);
  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    if(IS_SEEN)eng.log->addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) {isContentKnown_ = isTraitKnown_ = true;}
  Renderer::drawMapAndInterface();
  return true;
}

void Tomb::examine() {
  vector<PropId> props;
  Map::player->getPropHandler().getAllActivePropIds(props);

  if(find(props.begin(), props.end(), propConfused) != props.end()) {
    eng.log->addMsg("I start to search the tomb...");
    eng.log->addMsg("but I cannot grasp what for.");
    GameTime::actorDidAct();
  } else if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The tomb is empty.");
  } else {
    if(isTraitKnown_ == false && trait_ != TombTrait::endOfTombTraits) {
      const int FIND_ONE_IN_N = PlayerBon::hasTrait(Trait::perceptive) ? 2 :
                                (PlayerBon::hasTrait(Trait::observant) ? 3 : 6);

      isTraitKnown_ = Rnd::oneIn(FIND_ONE_IN_N);
    }

    if(isTraitKnown_) {
      switch(trait_) {
        case TombTrait::auraOfUnrest: {
          eng.log->addMsg("It has a certain aura of unrest about it.");
        } break;

        case TombTrait::forebodingCarvedSigns: {
          if(PlayerBon::getBg() == Bg::occultist) {
            eng.log->addMsg("There is a curse carved on the box.");
          } else {
            eng.log->addMsg("There are some ominous runes carved on the box.");
          }
        } break;

        case TombTrait::stench: {
          eng.log->addMsg("There is a pungent stench.");
        } break;

        case TombTrait::endOfTombTraits: {} break;
      }
    } else {
      eng.log->addMsg("I find nothing significant.");
    }
    GameTime::actorDidAct();
  }
}

//void Tomb::bash(Actor& actorTrying) {
//  (void)actorTrying;

//  const Inventory& inv = Map::player->getInv();
//  bool hasSledgehammer = false;
//  Item* item = inv.getItemInSlot(SlotId::wielded);
//  if(item != NULL) {
//    hasSledgehammer = item->getData().id == ItemId::sledgeHammer;
//  }
//  if(hasSledgehammer == false) {
//    item = inv.getItemInSlot(SlotId::wieldedAlt);
//    hasSledgehammer = item->getData().id == ItemId::sledgeHammer;
//  }
//  if(hasSledgehammer == false) {
//    hasSledgehammer = inv.hasItemInGeneral(ItemId::sledgeHammer);
//  }
//  if(hasSledgehammer) {
//    possibleActions.push_back(tombAction_smashLidWithSledgehammer);
//  }

//  eng.log->addMsg("I strike at the lid with a Sledgehammer.");
//  const int BREAK_N_IN_10 = IS_WEAK ? 1 : 8;
//  if(Rnd::fraction(BREAK_N_IN_10, 10)) {
//    eng.log->addMsg("The lid cracks open!");
//    if(IS_BLESSED == false && (IS_CURSED || Rnd::oneIn(3))) {
//      itemContainer_.destroySingleFragile();
//    }
//    open();
//  } else {
//    eng.log->addMsg("The lid resists.");
//  }

//  GameTime::actorDidAct();
//}

//void Tomb::disarm() {

//case tombAction_carveCurseWard: {
//    if(IS_CURSED == false && (IS_BLESSED || Rnd::fraction(4, 5))) {
//      eng.log->addMsg("The curse is cleared.");
//    } else {
//      eng.log->addMsg("I make a mistake, the curse is doubled!");
//      PropCursed* const curse =
//        new PropCursed(eng, propTurnsStd);
//      curse->turnsLeft_ *= 2;
//      propHlr.tryApplyProp(curse, true);
//    }
//    trait_ = endOfTombTraits;
//  } break;
//}

void Tomb::triggerTrap(Actor& actor) {
  (void)actor;

  vector<ActorId> actorCandidates;

  switch(trait_) {
    case TombTrait::auraOfUnrest: {
      const int DLVL = Map::getDlvl();

      for(int i = 1; i < endOfActorIds; i++) {
        const ActorData& d = ActorData::dataList[i];
        if(
          d.isGhost && d.isAutoSpawnAllowed && d.isUnique == false &&
          ((DLVL + 5) >= d.spawnMinDLVL || DLVL >= MIN_DLVL_NASTY_TRAPS)) {
          actorCandidates.push_back(ActorId(i));
        }
      }
      eng.log->addMsg("Something rises from the tomb!");
    } break;

    case TombTrait::forebodingCarvedSigns: {
      Map::player->getPropHandler().tryApplyProp(
        new PropCursed(eng, propTurnsStd));
    } break;

    case TombTrait::stench: {
      if(Rnd::coinToss()) {
        eng.log->addMsg("Fumes burst out from the tomb!");
        Prop* prop = NULL;
        SDL_Color fumeClr = clrMagenta;
        const int RND = Rnd::percentile();
        if(RND < 20) {
          prop = new PropPoisoned(eng, propTurnsStd);
          fumeClr = clrGreenLgt;
        } else if(RND < 40) {
          prop = new PropDiseased(eng, propTurnsSpecific, 50);
          fumeClr = clrGreen;
        } else {
          prop = new PropParalyzed(eng, propTurnsStd);
          prop->turnsLeft_ *= 2;
        }
        Explosion::runExplosionAt(
          pos_, eng, ExplType::applyProp, ExplSrc::misc, 0, SfxId::endOfSfxId,
          prop, &fumeClr);
      } else {
        for(int i = 1; i < endOfActorIds; i++) {
          const ActorData& d = ActorData::dataList[i];
          if(
            d.intrProps[propOoze] &&
            d.isAutoSpawnAllowed  &&
            d.isUnique == false) {
            actorCandidates.push_back(ActorId(i));
          }
        }
        eng.log->addMsg("Something creeps up from the tomb!");
      }

    } break;

    default: {} break;
  }

  if(actorCandidates.empty() == false) {
    const unsigned int ELEM = Rnd::range(0, actorCandidates.size() - 1);
    const ActorId actorIdToSpawn = actorCandidates.at(ELEM);
    Actor* const monster = eng.actorFactory->spawnActor(actorIdToSpawn, pos_);
    dynamic_cast<Monster*>(monster)->becomeAware(false);
  }
}

//--------------------------------------------------------- CHEST
Chest::Chest(FeatureId id, Pos pos) :
  FeatureStatic(id, pos), isContentKnown_(false),
  isLocked_(false), isTrapped_(false), isTrapStatusKnown_(false),
  material(ChestMtrl(Rnd::range(0, endOfChestMaterial - 1))) {

  const bool IS_TREASURE_HUNTER =
    PlayerBon::hasTrait(Trait::treasureHunter);
  const int NR_ITEMS_MIN = Rnd::oneIn(10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 3 : 2;
  itemContainer_.setRandomItemsForFeature(
    feature_chest, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  if(itemContainer_.items_.empty() == false) {
    isLocked_   = Rnd::fraction(6, 10);
    isTrapped_  = Rnd::fraction(6, 10);
  }
}

void Chest::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The chest is empty.");
    } else {
      if(isLocked_) {eng.log->addMsg("The chest is locked.");} else {open();}
      GameTime::actorDidAct();
    }
  }
}

void Chest::trySprainPlayer() {
  const int SPRAIN_ONE_IN_N = PlayerBon::hasTrait(Trait::rugged) ? 6 :
                              PlayerBon::hasTrait(Trait::tough)  ? 5 : 4;
  if(Rnd::oneIn(SPRAIN_ONE_IN_N)) {
    eng.log->addMsg("I sprain myself.", clrMsgBad);
    Map::player->hit(Rnd::range(1, 5), DmgType::pure, false);
  }
}

bool Chest::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;

  if(IS_SEEN) eng.log->addMsg("The chest opens.");
  triggerTrap(*Map::player);

  if(Map::player->deadState == ActorDeadState::alive) {
    if(itemContainer_.items_.empty()) {
      if(IS_SEEN) eng.log->addMsg("There is nothing of value inside.");
    } else {
      if(IS_SEEN) eng.log->addMsg("There are some items inside.");
      itemContainer_.dropItems(pos_, eng);
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

void Chest::bash(Actor& actorTrying) {
  (void)actorTrying;

  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The chest is empty.");
  } else {

    eng.log->addMsg("I kick the lid.");

    vector<PropId> props;
    Map::player->getPropHandler().getAllActivePropIds(props);

    if(
      find(props.begin(), props.end(), propWeakened) != props.end() ||
      material == chestMtrl_iron) {
      trySprainPlayer();
      eng.log->addMsg("It seems futile.");
    } else {

      const bool IS_CURSED =
        find(props.begin(), props.end(), propCursed)  != props.end();
      const bool IS_BLESSED =
        find(props.begin(), props.end(), propBlessed) != props.end();

      if(IS_BLESSED == false && (IS_CURSED || Rnd::oneIn(3))) {
        itemContainer_.destroySingleFragile();
      }

      const bool IS_TOUGH     = PlayerBon::hasTrait(Trait::tough);
      const bool IS_RUGGED    = PlayerBon::hasTrait(Trait::rugged);

      const int OPEN_ONE_IN_N = IS_RUGGED ? 2 : IS_TOUGH ? 3 : 5;

      if(Rnd::oneIn(OPEN_ONE_IN_N)) {
        eng.log->addMsg("I kick the lid open!");
        open();
      } else {
        eng.log->addMsg("The lock resists.");
        trySprainPlayer();
      }
    }
    GameTime::actorDidAct();
  }

  //TODO Force lock with weapon - remove or reimplement - how?
//      Inventory& inv    = Map::player->getInv();
//      Item* const item  = inv.getItemInSlot(SlotId::wielded);
//
//      if(item == NULL) {
//        eng.log->addMsg(
//          "I attempt to punch the lock open, nearly breaking my hand.",
//          clrMsgBad);
//        Map::player->hit(1, DmgType::pure, false);
//      } else {
//        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);
//
//        if(Rnd::percentile() < CHANCE_TO_DMG_WPN) {
//          const string wpnName = eng.itemDataHandler->getItemRef(
//                                   *item, ItemRefType::plain, true);
//
//          Weapon* const wpn = dynamic_cast<Weapon*>(item);
//
//          if(wpn->meleeDmgPlus == 0) {
//            eng.log->addMsg("My " + wpnName + " breaks!");
//            delete wpn;
//            inv.getSlot(SlotId::wielded)->item = NULL;
//          } else {
//            eng.log->addMsg("My " + wpnName + " is damaged!");
//            wpn->meleeDmgPlus--;
//          }
//          return;
//        }
//
//        if(IS_WEAK) {
//          eng.log->addMsg("It seems futile.");
//        } else {
//          const int CHANCE_TO_OPEN = 40;
//          if(Rnd::percentile() < CHANCE_TO_OPEN) {
//            eng.log->addMsg("I force the lock open!");
//            open();
//          } else {
//            eng.log->addMsg("The lock resists.");
//          }
//        }
//      }
}

void Chest::examine() {
  vector<PropId> props;
  Map::player->getPropHandler().getAllActivePropIds(props);

  if(find(props.begin(), props.end(), propConfused) != props.end()) {
    eng.log->addMsg("I start to search the chest...");
    eng.log->addMsg("but I cannot grasp the purpose.");
    GameTime::actorDidAct();
  } else if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The chest is empty.");
  } else {
    if(isLocked_) {
      eng.log->addMsg("The chest is locked.");
    }

    const int FIND_ONE_IN_N = PlayerBon::hasTrait(Trait::perceptive) ? 3 :
                              (PlayerBon::hasTrait(Trait::observant) ? 4 : 7);

    if(isTrapped_ && (isTrapStatusKnown_ || (Rnd::oneIn(FIND_ONE_IN_N)))) {
      eng.log->addMsg("There appears to be a hidden trap mechanism!");
      isTrapStatusKnown_ = true;
    } else {
      eng.log->addMsg("I find nothing unusual about it.");
    }
    GameTime::actorDidAct();
  }
}

void Chest::disarm() {
  //Try disarming trap
  if(isTrapped_ && isTrapStatusKnown_) {
    eng.log->addMsg("I attempt to disarm the trap.");

    const int TRIGGER_ONE_IN_N = 5;
    if(Rnd::oneIn(TRIGGER_ONE_IN_N)) {
      eng.log->addMsg("I set off the trap!");
      triggerTrap(*Map::player);
    } else {

      const int DISARM_ONE_IN_N = 2;

      if(Rnd::oneIn(DISARM_ONE_IN_N)) {
        eng.log->addMsg("I successfully disarm it!");
        isTrapped_ = false;
      } else {
        eng.log->addMsg("I failed to disarm it.");
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
      Map::getDlvl() >= MIN_DLVL_NASTY_TRAPS &&
      Rnd::oneIn(EXPLODE_ONE_IN_N)) {
      eng.log->addMsg("The trap explodes!");
      Explosion::runExplosionAt(pos_, eng, ExplType::expl, ExplSrc::misc, 0,
                                SfxId::explosion);
      if(Map::player->deadState == ActorDeadState::alive) {
        eng.featureFactory->spawnFeatureAt(feature_rubbleLow, pos_);
      }
    } else {
      eng.log->addMsg("Fumes burst out from the chest!");
      Prop* prop = NULL;
      SDL_Color fumeClr = clrMagenta;
      const int RND = Rnd::percentile();
      if(RND < 20) {
        prop = new PropPoisoned(eng, propTurnsStd);
        fumeClr = clrGreenLgt;
      } else if(RND < 40) {
        prop = new PropDiseased(eng, propTurnsStd);
        fumeClr = clrGreen;
      } else {
        prop = new PropParalyzed(eng, propTurnsStd);
        prop->turnsLeft_ *= 2;
      }
      Explosion::runExplosionAt(pos_, eng, ExplType::applyProp, ExplSrc::misc,
                                0, SfxId::endOfSfxId, prop, &fumeClr);
    }
  }
}

//--------------------------------------------------------- FOUNTAIN
Fountain::Fountain(FeatureId id, Pos pos) :
  FeatureStatic(id, pos), fountainType_(FountainType::tepid),
  fountainMaterial_(FountainMaterial::stone) {

  if(Rnd::oneIn(4)) {fountainMaterial_ = FountainMaterial::gold;}

  switch(fountainMaterial_) {
    case FountainMaterial::stone: {
      const int NR_TYPES = int(FountainType::endOfFountainTypes);
      fountainType_ = FountainType(Rnd::range(1, NR_TYPES - 1));
    } break;

    case FountainMaterial::gold: {
      vector<FountainType> typeCandidates {
        FountainType::bless, FountainType::refreshing, FountainType::spirit,
        FountainType::vitality, FountainType::rFire, FountainType::rCold,
        FountainType::rElec, FountainType::rFear, FountainType::rConfusion
      };
      const int ELEMENT = Rnd::range(0, typeCandidates.size() - 1);
      fountainType_ = typeCandidates.at(ELEMENT);
    } break;
  }
}

SDL_Color Fountain::getClr() const {
  switch(fountainMaterial_) {
    case FountainMaterial::stone: return clrGray;
    case FountainMaterial::gold:  return clrYellow;
  }
  assert("Failed to get fountain color" && false);
  return clrBlack;
}

string Fountain::getDescr(const bool DEFINITE_ARTICLE) const {
  string article = DEFINITE_ARTICLE ? "the" : "a";

  switch(fountainMaterial_) {
    case FountainMaterial::stone: return article + " stone fountain";
    case FountainMaterial::gold:  return article + " golden fountain";
  }
  assert("Failed to get fountain description" && false);
  return "";
}

void Fountain::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {

    if(fountainType_ == FountainType::dry) {
      eng.log->addMsg("The fountain is dried out.");
    } else {
      PropHandler& propHlr = Map::player->getPropHandler();

      eng.log->addMsg("I drink from the fountain...");

      switch(fountainType_) {
        case FountainType::dry: {} break;

        case FountainType::tepid: {
          eng.log->addMsg("The water is tepid.");
        } break;

        case FountainType::refreshing: {
          eng.log->addMsg("It's very refreshing.");
          Map::player->restoreHp(1, false);
          Map::player->restoreSpi(1, false);
          Map::player->restoreShock(5, false);
        } break;

        case FountainType::bless: {
          propHlr.tryApplyProp(new PropBlessed(eng, propTurnsStd));
        } break;

        case FountainType::curse: {
          propHlr.tryApplyProp(new PropCursed(eng, propTurnsStd));
        } break;

        case FountainType::spirit: {
          Map::player->restoreSpi(2, true, true);
        } break;

        case FountainType::vitality: {
          Map::player->restoreHp(2, true, true);
        } break;

        case FountainType::disease: {
          propHlr.tryApplyProp(new PropDiseased(eng, propTurnsSpecific, 50));
        } break;

        case FountainType::poison: {
          propHlr.tryApplyProp(new PropPoisoned(eng, propTurnsStd));
        } break;

        case FountainType::frenzy: {
          propHlr.tryApplyProp(new PropFrenzied(eng, propTurnsStd));
        } break;

        case FountainType::paralyze: {
          propHlr.tryApplyProp(new PropParalyzed(eng, propTurnsStd));
        } break;

        case FountainType::blind: {
          propHlr.tryApplyProp(new PropBlind(eng, propTurnsStd));
        } break;

        case FountainType::faint: {
          propHlr.tryApplyProp(new PropFainted(eng, propTurnsSpecific, 10));
        } break;

        case FountainType::rFire: {
          propHlr.tryApplyProp(new PropRFire(eng, propTurnsStd));
        } break;

        case FountainType::rCold: {
          propHlr.tryApplyProp(new PropRCold(eng, propTurnsStd));
        } break;

        case FountainType::rElec: {
          propHlr.tryApplyProp(new PropRElec(eng, propTurnsStd));
        } break;

        case FountainType::rConfusion: {
          propHlr.tryApplyProp(new PropRConfusion(eng, propTurnsStd));
        } break;

        case FountainType::rFear: {
          propHlr.tryApplyProp(new PropRFear(eng, propTurnsStd));
        } break;

        case FountainType::endOfFountainTypes: {} break;
      }

      if(Rnd::oneIn(5)) {
        eng.log->addMsg("The fountain dries out.");
        fountainType_ = FountainType::dry;
      }
    }
  }
  GameTime::actorDidAct();
}


//--------------------------------------------------------- CABINET
Cabinet::Cabinet(FeatureId id, Pos pos) :
  FeatureStatic(id, pos), isContentKnown_(false) {

  const bool IS_TREASURE_HUNTER =
    PlayerBon::hasTrait(Trait::treasureHunter);
  const int IS_EMPTY_N_IN_10 = 5;
  const int NR_ITEMS_MIN = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 2 : 1;
  itemContainer_.setRandomItemsForFeature(
    feature_cabinet, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cabinet::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {

    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The cabinet is empty.");
    } else {
      open();
    }
  }
}

bool Cabinet::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;

  if(IS_SEEN) eng.log->addMsg("The cabinet opens.");

  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    if(IS_SEEN) eng.log->addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) isContentKnown_ = true;
  Renderer::drawMapAndInterface(true);
  return true;
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(FeatureId id, Pos pos) :
  FeatureStatic(id, pos), isContentKnown_(false) {

  const bool IS_TREASURE_HUNTER =
    PlayerBon::hasTrait(Trait::treasureHunter);
  const int IS_EMPTY_N_IN_10 = 6;
  const int NR_ITEMS_MIN = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);
  itemContainer_.setRandomItemsForFeature(
    feature_cocoon, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cocoon::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The cocoon is empty.");
    } else {
      open();
    }
  }
}

void Cocoon::triggerTrap(Actor& actor) {
  (void)actor;

  const int RND = Rnd::percentile();

  if(RND < 15) {
    eng.log->addMsg("There is a half-dissolved human body inside!");
    Map::player->incrShock(ShockValue::shockValue_heavy, ShockSrc::misc);
  } else if(RND < 50) {
    trace << "Cocoon: Attempting to spawn spiders" << endl;
    vector<ActorId> spawnCandidates;
    for(unsigned int i = 1; i < endOfActorIds; i++) {
      const ActorData& d = ActorData::dataList[i];
      if(d.isSpider && d.actorSize == actorSize_floor &&
          d.isAutoSpawnAllowed && d.isUnique == false) {
        spawnCandidates.push_back(d.id);
      }
    }

    const int NR_CANDIDATES = spawnCandidates.size();
    if(NR_CANDIDATES > 0) {
      trace << "Cocoon: Spawn candidates found, attempting to place" << endl;
      eng.log->addMsg("There are spiders inside!");
      const int NR_SPIDERS = Rnd::range(2, 5);
      const int ELEMENT = Rnd::range(0, NR_CANDIDATES - 1);
      const ActorId actorIdToSummon = spawnCandidates.at(ELEMENT);
      eng.actorFactory->summonMonsters(
        pos_, vector<ActorId>(NR_SPIDERS, actorIdToSummon), true);
    }
  }
}

bool Cocoon::open() {
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  if(IS_SEEN) eng.log->addMsg("The cocoon opens.");
  triggerTrap(*Map::player);
  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    if(IS_SEEN) eng.log->addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) isContentKnown_ = true;
  Renderer::drawMapAndInterface(true);

  return true;
}

//--------------------------------------------------------- ALTAR
//Altar::Altar(FeatureId id, Pos pos) :
//  FeatureStatic(id, pos, eng) {}
//
//void Altar::featureSpecific_examine() {
//}

//--------------------------------------------------------- CARVED PILLAR
//CarvedPillar::CarvedPillar(FeatureId id, Pos pos) :
//  FeatureStatic(id, pos, eng) {}
//
//void CarvedPillar::featureSpecific_examine() {
//}

//--------------------------------------------------------- BARREL
//Barrel::Barrel(FeatureId id, Pos pos) :
//  FeatureStatic(id, pos, eng) {}
//
//void Barrel::featureSpecific_examine() {
//}
