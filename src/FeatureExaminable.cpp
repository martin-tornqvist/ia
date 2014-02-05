#include "FeatureExaminable.h"

#include "Engine.h"
#include "Log.h"
#include "Renderer.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemFactory.h"
#include "ItemDrop.h"
#include "Popup.h"
#include "PlayerBonuses.h"
#include "ActorPlayer.h"
#include "Inventory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Explosion.h"
#include "PopulateMonsters.h"
#include "Map.h"
#include "FeatureFactory.h"

//---------------------------------------------------------ITEM CONTAINE
ItemContainerFeature::ItemContainerFeature() {items_.resize(0);}

ItemContainerFeature::~ItemContainerFeature() {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
}

void ItemContainerFeature::setRandomItemsForFeature(
  const Feature_t featureId,
  const int NR_ITEMS_TO_ATTEMPT, Engine& engine) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
  items_.resize(0);

  if(NR_ITEMS_TO_ATTEMPT > 0) {
    while(items_.empty()) {
      vector<ItemId_t> itemCandidates;
      for(unsigned int i = 1; i < endOfItemIds; i++) {
        ItemData* const curData = engine.itemDataHandler->dataList[i];
        for(
          unsigned int ii = 0;
          ii < curData->featuresCanBeFoundIn.size();
          ii++) {
          pair<Feature_t, int> featuresFoundIn =
            curData->featuresCanBeFoundIn.at(ii);
          if(featuresFoundIn.first == featureId) {
            if(engine.dice.percentile() < featuresFoundIn.second) {
              if(
                engine.dice.percentile() <
                curData->chanceToIncludeInSpawnList) {
                itemCandidates.push_back(static_cast<ItemId_t>(i));
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
            engine.dice.range(0, NR_CANDIDATES - 1);
          Item* item =
            engine.itemFactory->spawnItem(itemCandidates.at(ELEMENT));
          engine.itemFactory->setItemRandomizedProperties(item);
          items_.push_back(item);
        }
      }
    }
  }
}

void ItemContainerFeature::dropItems(const Pos& pos, Engine& engine) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    engine.itemDrop->dropItemOnMap(pos, *items_.at(i));
  }
  items_.resize(0);
}

void ItemContainerFeature::destroySingleFragile(Engine& engine) {
  //TODO Generalize this function (perhaps isFragile variable in item data)

  for(unsigned int i = 0; i < items_.size(); i++) {
    Item* const item = items_.at(i);
    const ItemData& d = item->getData();
    if(d.isPotion || d.id == item_molotov) {
      delete item;
      items_.erase(items_.begin() + i);
      engine.log->addMsg("I hear a muffled shatter.");
      break;
    }
  }
}

//--------------------------------------------------------- TOMB
Tomb::Tomb(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine), isContentKnown_(false),
  isTraitKnown_(false), pushLidOneInN_(engine.dice.range(6, 14)),
  appearance_(tombAppearance_common), trait_(endOfTombTraits) {

  //Contained items
  PlayerBonHandler* const bonHlr = eng.playerBonHandler;
  const int NR_ITEMS_MIN = eng.dice.oneIn(3) ? 0 : 1;
  const int NR_ITEMS_MAX =
    NR_ITEMS_MIN + (bonHlr->hasTrait(traitTreasureHunter) ? 1 : 0);

  itemContainer_.setRandomItemsForFeature(
    feature_tomb, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  //Exterior appearance
  if(engine.dice.oneIn(5)) {
    appearance_ =
      TombAppearance_t(engine.dice.range(0, endOfTombAppearance - 1));
  } else {
    for(Item * item : itemContainer_.items_) {
      const ItemValue_t itemValue = item->getData().itemValue;
      if(itemValue == itemValue_majorTreasure) {
        appearance_ = tombAppearance_marvelous;
        break;
      } else if(itemValue == itemValue_minorTreasure) {
        appearance_ = tombAppearance_impressive;
      }
    }
  }

  const bool IS_CONTAINING_ITEMS = itemContainer_.items_.empty() == false;

  if(IS_CONTAINING_ITEMS) {
    const int RND = engine.dice.percentile();
    if(RND < 15) {
      trait_ = tombTrait_forebodingCarvedSigns;
    } else if(RND < 45) {
      trait_ = tombTrait_stench;
    } else if(RND < 75) {
      trait_ = tombTrait_auraOfUnrest;
    }
    trace << "Tomb: Set trait (" << trait_ << ")" << endl;
  }
}

void Tomb::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The tomb is empty.");
    } else {
      eng.log->addMsg("I attempt to push the lid.");

      vector<PropId_t> props;
      eng.player->getPropHandler().getAllActivePropIds(props);

      if(find(props.begin(), props.end(), propWeakened) != props.end()) {
        trySprainPlayer();
        eng.log->addMsg("It seems futile.");
      } else {
        const PlayerBonHandler* const bonHlr = eng.playerBonHandler;
        const int BON = bonHlr->hasTrait(traitRugged) ? 8 :
                        bonHlr->hasTrait(traitTough)  ? 4 : 0;

        trace << "Tomb: Base chance to push lid is: 1 in ";
        trace << pushLidOneInN_ << endl;

        trace << "Tomb: Bonus to roll: " << BON << endl;

        const int ROLL_TOT = eng.dice.range(1, pushLidOneInN_) + BON;

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
      eng.gameTime->actorDidAct();
    }
  }
}

void Tomb::trySprainPlayer() {
  const PlayerBonHandler* const bonHlr = eng.playerBonHandler;
  const int SPRAIN_ONE_IN_N = bonHlr->hasTrait(traitRugged) ? 6 :
                              bonHlr->hasTrait(traitTough)  ? 5 : 4;
  if(eng.dice.oneIn(SPRAIN_ONE_IN_N)) {
    eng.log->addMsg("I sprain myself.", clrMsgBad);
    eng.player->hit(eng.dice.range(1, 5), dmgType_pure, false);
  }
}

bool Tomb::open() {
  const bool IS_SEEN = eng.map->cells[pos_.x][pos_.y].isSeenByPlayer;
  if(IS_SEEN) {
    eng.log->addMsg("The lid comes off!");
    eng.log->addMsg("The tomb opens.");
  }
  triggerTrap(*eng.player);
  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    if(IS_SEEN)eng.log->addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) {isContentKnown_ = isTraitKnown_ = true;}
  eng.renderer->drawMapAndInterface();
  return true;
}

void Tomb::examine() {
  vector<PropId_t> props;
  eng.player->getPropHandler().getAllActivePropIds(props);

  if(find(props.begin(), props.end(), propConfused) != props.end()) {
    eng.log->addMsg("I start to search the tomb...");
    eng.log->addMsg("but I cannot grasp the purpose.");
    eng.gameTime->actorDidAct();
  } else if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The tomb is empty.");
  } else {
    switch(appearance_) {
      case tombAppearance_common: {
        eng.log->addMsg("It looks ordinary.");
      } break;

      case tombAppearance_impressive: {
        eng.log->addMsg("It looks impressive.");
      } break;

      case tombAppearance_marvelous: {
        eng.log->addMsg("It looks marvelous!");
      } break;

      case endOfTombAppearance: {} break;
    }

    if(isTraitKnown_ == false && trait_ != endOfTombTraits) {
      const PlayerBonHandler& bonHlr = *eng.playerBonHandler;
      const int FIND_ONE_IN_N = bonHlr.hasTrait(traitPerceptive) ? 2 :
                                (bonHlr.hasTrait(traitObservant) ? 3 : 6);

      isTraitKnown_ = eng.dice.oneIn(FIND_ONE_IN_N);
    }

    if(isTraitKnown_) {
      switch(trait_) {
        case tombTrait_auraOfUnrest: {
          eng.log->addMsg("It has a certain aura of unrest about it.");
        } break;

        case tombTrait_forebodingCarvedSigns: {
          if(eng.playerBonHandler->getBg() == bgOccultist) {
            eng.log->addMsg("There is a curse carved on the box.");
          } else {
            eng.log->addMsg("There are some ominous runes carved on the box.");
          }
        } break;

        case tombTrait_stench: {
          eng.log->addMsg("There is a pungent stench.");
        } break;

        case endOfTombTraits: {} break;
      }
    } else {
      eng.log->addMsg("I find nothing significant.");
    }
    eng.gameTime->actorDidAct();
  }
}

//void Tomb::bash(Actor& actorTrying) {
//  (void)actorTrying;

//  const Inventory& inv = eng.player->getInv();
//  bool hasSledgehammer = false;
//  Item* item = inv.getItemInSlot(slot_wielded);
//  if(item != NULL) {
//    hasSledgehammer = item->getData().id == item_sledgeHammer;
//  }
//  if(hasSledgehammer == false) {
//    item = inv.getItemInSlot(slot_wieldedAlt);
//    hasSledgehammer = item->getData().id == item_sledgeHammer;
//  }
//  if(hasSledgehammer == false) {
//    hasSledgehammer = inv.hasItemInGeneral(item_sledgeHammer);
//  }
//  if(hasSledgehammer) {
//    possibleActions.push_back(tombAction_smashLidWithSledgehammer);
//  }

//  eng.log->addMsg("I strike at the lid with a Sledgehammer.");
//  const int BREAK_N_IN_10 = IS_WEAK ? 1 : 8;
//  if(eng.dice.fraction(BREAK_N_IN_10, 10)) {
//    eng.log->addMsg("The lid cracks open!");
//    if(IS_BLESSED == false && (IS_CURSED || eng.dice.oneIn(3))) {
//      itemContainer_.destroySingleFragile(eng);
//    }
//    open();
//  } else {
//    eng.log->addMsg("The lid resists.");
//  }

//  eng.gameTime->actorDidAct();
//}

//void Tomb::disarm() {

//case tombAction_carveCurseWard: {
//    if(IS_CURSED == false && (IS_BLESSED || eng.dice.fraction(4, 5))) {
//      eng.log->addMsg("The curse is cleared.");
//    } else {
//      eng.log->addMsg("I make a misstake, the curse is doubled!");
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

  vector<ActorId_t> actorCandidates;

  switch(trait_) {
    case tombTrait_auraOfUnrest: {
      for(int i = 1; i < endOfActorIds; i++) {
        const ActorData& d = eng.actorDataHandler->dataList[i];
        if(d.isGhost && d.isAutoSpawnAllowed && d.isUnique == false) {
          actorCandidates.push_back(static_cast<ActorId_t>(i));
        }
      }
      eng.log->addMsg("Something rises from the tomb!");
    } break;

    case tombTrait_forebodingCarvedSigns: {
      eng.player->getPropHandler().tryApplyProp(
        new PropCursed(eng, propTurnsStd));
    } break;

    case tombTrait_stench: {
      if(eng.dice.coinToss()) {
        eng.log->addMsg("Fumes burst out from the tomb!");
        Prop* prop = NULL;
        SDL_Color fumeClr = clrMagenta;
        const int RND = eng.dice.percentile();
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
        Explosion::runExplosionAt(
          pos_, eng, 0, endOfSfx, false, prop, true, fumeClr);
      } else {
        for(unsigned int i = 1; i < endOfActorIds; i++) {
          const ActorData& d = eng.actorDataHandler->dataList[i];
          if(
            d.intrProps[propOoze] &&
            d.isAutoSpawnAllowed  &&
            d.isUnique == false) {
            actorCandidates.push_back(static_cast<ActorId_t>(i));
          }
        }
        eng.log->addMsg("Something creeps up from the tomb!");
      }

    } break;

    default: {} break;
  }

  if(actorCandidates.empty() == false) {
    const unsigned int ELEM = eng.dice.range(0, actorCandidates.size() - 1);
    const ActorId_t actorIdToSpawn = actorCandidates.at(ELEM);
    Actor* const monster = eng.actorFactory->spawnActor(actorIdToSpawn, pos_);
    dynamic_cast<Monster*>(monster)->becomeAware();
  }
}

//--------------------------------------------------------- CHEST
Chest::Chest(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine), isContentKnown_(false),
  isLocked_(false), isTrapped_(false), isTrapStatusKnown_(false),
  material(ChestMtrl_t(engine.dice.range(0, endOfChestMaterial - 1))) {

  PlayerBonHandler* const bonHlr = eng.playerBonHandler;
  const bool IS_TREASURE_HUNTER =
    bonHlr->hasTrait(traitTreasureHunter);
  const int NR_ITEMS_MIN = eng.dice.oneIn(10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 3 : 2;
  itemContainer_.setRandomItemsForFeature(
    feature_chest, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  if(itemContainer_.items_.empty() == false) {
    isLocked_   = eng.dice.fraction(6, 10);
    isTrapped_  = eng.dice.fraction(6, 10);
  }
}

void Chest::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The chest is empty.");
    } else {
      if(isLocked_) {eng.log->addMsg("The chest is locked.");} else {open();}
      eng.gameTime->actorDidAct();
    }
  }
}

void Chest::trySprainPlayer() {
  const PlayerBonHandler* const bonHlr = eng.playerBonHandler;
  const int SPRAIN_ONE_IN_N = bonHlr->hasTrait(traitRugged) ? 6 :
                              bonHlr->hasTrait(traitTough)  ? 5 : 4;
  if(eng.dice.oneIn(SPRAIN_ONE_IN_N)) {
    eng.log->addMsg("I sprain myself.", clrMsgBad);
    eng.player->hit(eng.dice.range(1, 5), dmgType_pure, false);
  }
}

bool Chest::open() {
  const bool IS_SEEN = eng.map->cells[pos_.x][pos_.y].isSeenByPlayer;

  if(IS_SEEN) eng.log->addMsg("The chest opens.");
  triggerTrap(*eng.player);

  if(eng.player->deadState == actorDeadState_alive) {
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
  eng.renderer->drawMapAndInterface();
  return true;
}

void Chest::bash(Actor& actorTrying) {
  (void)actorTrying;

  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The chest is empty.");
  } else {

    eng.log->addMsg("I kick the lid.");

    vector<PropId_t> props;
    eng.player->getPropHandler().getAllActivePropIds(props);

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

      if(IS_BLESSED == false && (IS_CURSED || eng.dice.oneIn(3))) {
        itemContainer_.destroySingleFragile(eng);
      }

      PlayerBonHandler* const bonHlr = eng.playerBonHandler;
      const bool IS_TOUGH     = bonHlr->hasTrait(traitTough);
      const bool IS_RUGGED    = bonHlr->hasTrait(traitRugged);

      const int OPEN_ONE_IN_N = IS_RUGGED ? 2 : IS_TOUGH ? 3 : 5;

      if(eng.dice.oneIn(OPEN_ONE_IN_N)) {
        eng.log->addMsg("I kick the lid open!");
        open();
      } else {
        eng.log->addMsg("The lock resists.");
        trySprainPlayer();
      }
    }
    eng.gameTime->actorDidAct();
  }

  //TODO Force lock with weapon - remove or reimplement - how?
//      Inventory& inv    = eng.player->getInv();
//      Item* const item  = inv.getItemInSlot(slot_wielded);
//
//      if(item == NULL) {
//        eng.log->addMsg(
//          "I attempt to punch the lock open, nearly breaking my hand.",
//          clrMsgBad);
//        eng.player->hit(1, dmgType_pure, false);
//      } else {
//        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);
//
//        if(eng.dice.percentile() < CHANCE_TO_DMG_WPN) {
//          const string wpnName = eng.itemDataHandler->getItemRef(
//                                   *item, itemRef_plain, true);
//
//          Weapon* const wpn = dynamic_cast<Weapon*>(item);
//
//          if(wpn->meleeDmgPlus == 0) {
//            eng.log->addMsg("My " + wpnName + " breaks!");
//            delete wpn;
//            inv.getSlot(slot_wielded)->item = NULL;
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
//          if(eng.dice.percentile() < CHANCE_TO_OPEN) {
//            eng.log->addMsg("I force the lock open!");
//            open();
//          } else {
//            eng.log->addMsg("The lock resists.");
//          }
//        }
//      }
}

void Chest::examine() {
  vector<PropId_t> props;
  eng.player->getPropHandler().getAllActivePropIds(props);

  if(find(props.begin(), props.end(), propConfused) != props.end()) {
    eng.log->addMsg("I start to search the chest...");
    eng.log->addMsg("but I cannot grasp the purpose.");
    eng.gameTime->actorDidAct();
  } else if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The chest is empty.");
  } else {
    if(isLocked_) {
      eng.log->addMsg("The chest is locked.");
    }

    PlayerBonHandler* const bonHlr = eng.playerBonHandler;

    const int FIND_ONE_IN_N = bonHlr->hasTrait(traitPerceptive) ? 3 :
                              (bonHlr->hasTrait(traitObservant) ? 4 : 7);

    if(isTrapped_ && (isTrapStatusKnown_ || (eng.dice.oneIn(FIND_ONE_IN_N)))) {
      eng.log->addMsg("There appears to be a hidden trap mechanism!");
      isTrapStatusKnown_ = true;
    } else {
      eng.log->addMsg("I find nothing unusual about it.");
    }
    eng.gameTime->actorDidAct();
  }
}

void Chest::disarm() {
  //Try disarming trap
  if(isTrapped_ && isTrapStatusKnown_) {
    eng.log->addMsg("I attempt to disarm the trap.");

    const int TRIGGER_ONE_IN_N = 5;
    if(eng.dice.oneIn(TRIGGER_ONE_IN_N)) {
      eng.log->addMsg("I set off the trap!");
      triggerTrap(*eng.player);
    } else {

      const int DISARM_ONE_IN_N = 2;

      if(eng.dice.oneIn(DISARM_ONE_IN_N)) {
        eng.log->addMsg("I successfully disarm it!");
        isTrapped_ = false;
      } else {
        eng.log->addMsg("I failed to disarm it.");
      }
    }
    eng.gameTime->actorDidAct();
    return;
  }


  //Try picking the lock //TODO Implement
//  if(isLocked_) {
//      eng.gameTime->actorDidAct();
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
      eng.map->getDlvl() >= MIN_DLVL_NASTY_TRAPS &&
      eng.dice.oneIn(EXPLODE_ONE_IN_N)) {
      eng.log->addMsg("The trap explodes!");
      Explosion::runExplosionAt(pos_, eng, 0, sfxExplosion, true);
      if(eng.player->deadState == actorDeadState_alive) {
        eng.featureFactory->spawnFeatureAt(feature_rubbleLow, pos_);
      }
    } else {
      eng.log->addMsg("Fumes burst out from the chest!");
      Prop* prop = NULL;
      SDL_Color fumeClr = clrMagenta;
      const int RND = eng.dice.percentile();
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
      Explosion::runExplosionAt(
        pos_, eng, 0, endOfSfx, false, prop, true, fumeClr);
    }
  }
}

//--------------------------------------------------------- FOUNTAIN
Fountain::Fountain(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine), fountainType(fountainTypeTepid) {

  fountainType = FountainType_t(eng.dice.range(1, endOfFountainTypes - 1));
}

void Fountain::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {

    if(fountainType == fountainTypeDry) {
      eng.log->addMsg("The fountain is dried out.");
    } else {
      PropHandler& propHlr = eng.player->getPropHandler();

      eng.log->addMsg("I drink from the fountain.");

      switch(fountainType) {
        case fountainTypeDry: {} break;

        case fountainTypeTepid: {
          eng.log->addMsg("The water is tepid.");
        } break;

        case fountainTypeRefreshing: {
          eng.log->addMsg("It's very refreshing.");
          eng.player->restoreHp(1, false);
          eng.player->restoreSpi(1, false);
          eng.player->restoreShock(5, false);
        } break;

        case fountainTypeBlessed: {
          propHlr.tryApplyProp(
            new PropBlessed(eng, propTurnsStd));
        } break;

        case fountainTypeCursed: {
          propHlr.tryApplyProp(
            new PropCursed(eng, propTurnsStd));
        } break;

        case fountainTypeSpirited: {
          eng.player->restoreSpi(2, true, true);
        } break;

        case fountainTypeVitality: {
          eng.player->restoreHp(2, true, true);
        } break;

        case fountainTypeDiseased: {
          propHlr.tryApplyProp(
            new PropDiseased(eng, propTurnsStd));
        } break;

        case fountainTypePoisoned: {
          propHlr.tryApplyProp(
            new PropPoisoned(eng, propTurnsStd));
        } break;

        case fountainTypeFrenzy: {
          propHlr.tryApplyProp(
            new PropFrenzied(eng, propTurnsStd));
        } break;

        case endOfFountainTypes: {} break;
      }

      if(eng.dice.oneIn(5)) {
        eng.log->addMsg("The fountain dries out.");
        fountainType = fountainTypeDry;
      }
    }
  }
}


//--------------------------------------------------------- CABINET
Cabinet::Cabinet(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine), isContentKnown_(false) {

  PlayerBonHandler* const bonHlr = eng.playerBonHandler;
  const bool IS_TREASURE_HUNTER =
    bonHlr->hasTrait(traitTreasureHunter);
  const int IS_EMPTY_N_IN_10 = 5;
  const int NR_ITEMS_MIN = eng.dice.fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 2 : 1;
  itemContainer_.setRandomItemsForFeature(
    feature_cabinet, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cabinet::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {

    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The cabinet is empty.");
    } else {
      open();
    }
  }
}

bool Cabinet::open() {
  const bool IS_SEEN = eng.map->cells[pos_.x][pos_.y].isSeenByPlayer;

  if(IS_SEEN) eng.log->addMsg("The cabinet opens.");

  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    if(IS_SEEN) eng.log->addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) isContentKnown_ = true;
  eng.renderer->drawMapAndInterface(true);
  return true;
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine), isContentKnown_(false) {

  PlayerBonHandler* const bonHlr = eng.playerBonHandler;
  const bool IS_TREASURE_HUNTER =
    bonHlr->hasTrait(traitTreasureHunter);
  const int IS_EMPTY_N_IN_10 = 6;
  const int NR_ITEMS_MIN = eng.dice.fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);
  itemContainer_.setRandomItemsForFeature(
    feature_cocoon, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cocoon::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {
    if(itemContainer_.items_.empty() && isContentKnown_) {
      eng.log->addMsg("The cocoon is empty.");
    } else {
      open();
    }
  }
}

void Cocoon::triggerTrap(Actor& actor) {
  (void)actor;

  const int RND = eng.dice.percentile();

  if(RND < 15) {
    eng.log->addMsg("There is a half-dissolved human body inside!");
    eng.player->incrShock(shockValue_heavy, shockSrc_misc);
  } else if(RND < 50) {
    trace << "Cocoon: Attempting to spawn spiders" << endl;
    vector<ActorId_t> spawnCandidates;
    for(unsigned int i = 1; i < endOfActorIds; i++) {
      const ActorData& d = eng.actorDataHandler->dataList[i];
      if(d.isSpider && d.actorSize == actorSize_floor &&
          d.isAutoSpawnAllowed && d.isUnique == false) {
        spawnCandidates.push_back(d.id);
      }
    }

    const int NR_CANDIDATES = spawnCandidates.size();
    if(NR_CANDIDATES > 0) {
      trace << "Cocoon: Spawn candidates found, attempting to place" << endl;
      eng.log->addMsg("There are spiders inside!");
      const int NR_SPIDERS = eng.dice.range(2, 5);
      const int ELEMENT = eng.dice.range(0, NR_CANDIDATES - 1);
      const ActorId_t actorIdToSummon = spawnCandidates.at(ELEMENT);
      eng.actorFactory->summonMonsters(
        pos_, vector<ActorId_t>(NR_SPIDERS, actorIdToSummon), true);
    }
  }
}

bool Cocoon::open() {
  const bool IS_SEEN = eng.map->cells[pos_.x][pos_.y].isSeenByPlayer;
  if(IS_SEEN) eng.log->addMsg("The cocoon opens.");
  triggerTrap(*eng.player);
  if(itemContainer_.items_.size() > 0) {
    if(IS_SEEN) eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    if(IS_SEEN) eng.log->addMsg("There is nothing of value inside.");
  }
  if(IS_SEEN) isContentKnown_ = true;
  eng.renderer->drawMapAndInterface(true);

  return true;
}

//--------------------------------------------------------- ALTAR
//Altar::Altar(Feature_t id, Pos pos, Engine& engine) :
//  FeatureStatic(id, pos, eng) {}
//
//void Altar::featureSpecific_examine() {
//}

//--------------------------------------------------------- CARVED PILLAR
//CarvedPillar::CarvedPillar(Feature_t id, Pos pos, Engine& engine) :
//  FeatureStatic(id, pos, eng) {}
//
//void CarvedPillar::featureSpecific_examine() {
//}

//--------------------------------------------------------- BARREL
//Barrel::Barrel(Feature_t id, Pos pos, Engine& engine) :
//  FeatureStatic(id, pos, eng) {}
//
//void Barrel::featureSpecific_examine() {
//}
