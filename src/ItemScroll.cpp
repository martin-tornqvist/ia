#include "ItemScroll.h"

#include "ItemData.h"
#include "Engine.h"
#include "StatusEffects.h"
#include "Log.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "DungeonClimb.h"
#include "FeatureTrap.h"
#include "Postmortem.h"
#include "Query.h"
#include "PlayerBonuses.h"
#include "Marker.h"
#include "Inventory.h"
#include "ItemPotion.h"
#include "ItemDevice.h"
#include "Popup.h"
#include "ItemArmor.h"
#include "ItemWeapon.h"
#include "PlayerPowersHandler.h"

const int BLAST_ANIMATION_DELAY_FACTOR = 2;

void ScrollOfMayhem::specificRead(Engine* const engine) {
  const int NR_OF_SWEEPS = 5;

  engine->log->addMessage("Destruction rages around me!");

  const int PLAYER_X = engine->player->pos.x;
  const int PLAYER_Y = engine->player->pos.y;

  const int AREA_RADI = 8;

  for(int i = 0; i < NR_OF_SWEEPS; i++) {
    for(int y = max(1, PLAYER_Y - AREA_RADI); y < min(MAP_Y_CELLS - 1, PLAYER_Y + AREA_RADI); y++) {
      for(int x = max(1, PLAYER_X - AREA_RADI); x < min(MAP_X_CELLS - 1, PLAYER_X + AREA_RADI); x++) {
        const coord c(x, y);
        bool isAdjToWalkableCell = false;
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            if(engine->map->featuresStatic[x + dx][y + dy]->isMoveTypePassable(moveType_walk)) {
              isAdjToWalkableCell = true;
            }
          }
        }
        if(isAdjToWalkableCell) {
          const int CHANCE_TO_DESTROY = 10;
          if(engine->dice.percentile() < CHANCE_TO_DESTROY) {
            engine->map->switchToDestroyedFeatAt(c);
          }
        }
      }
    }
  }

  for(int y = max(1, PLAYER_Y - AREA_RADI); y < min(MAP_Y_CELLS - 1, PLAYER_Y + AREA_RADI); y++) {
    for(int x = max(1, PLAYER_X - AREA_RADI); x < min(MAP_X_CELLS - 1, PLAYER_X + AREA_RADI); x++) {
      if(engine->map->featuresStatic[x][y]->canHaveBlood()) {
        const int CHANCE_FOR_BLOOD = 10;
        if(engine->dice.percentile() < CHANCE_FOR_BLOOD) {
          engine->map->featuresStatic[x][y]->setHasBlood(true);
        }
      }
    }
  }

  for(unsigned int i = 0; i < engine->gameTime->getLoopSize(); i++) {
    Actor* actor = engine->gameTime->getActorAt(i);
    if(actor != engine->player) {
      if(engine->player->checkIfSeeActor(*actor, NULL)) {
        actor->getStatusEffectsHandler()->tryAddEffect(new StatusBurning(engine));
      }
    }
  }

  engine->soundEmitter->emitSound(Sound("", true, engine->player->pos, true, true));

  setRealDefinitionNames(engine, false);
}

void ScrollOfPestilence::specificRead(Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

  const int RADI = 4;
  const int x0 = max(0, engine->player->pos.x - RADI);
  const int y0 = max(0, engine->player->pos.y - RADI);
  const int x1 = min(MAP_X_CELLS - 1, engine->player->pos.x + RADI);
  const int y1 = min(MAP_Y_CELLS - 1, engine->player->pos.y + RADI);

  ActorId_t monsterId = endOfActorIds;
  Dice& dice = engine->dice;

  for(int x = x0; x <= x1; x++) {
    for(int y = y0; y <= y1; y++) {
      if(blockers[x][y] == false) {
        monsterId = dice(1, 3) == 1 ?
                    (dice.coinToss() ? actor_greenSpider :
                     (dice.coinToss() ? actor_whiteSpider : actor_redSpider)) :
                      actor_rat;
        engine->actorFactory->spawnActor(monsterId, coord(x, y));
      }
    }
  }

  engine->log->addMessage("Disgusting critters appear around me!");
  setRealDefinitionNames(engine, false);
}

void ScrollOfDescent::specificRead(Engine* const engine) {
  if(engine->map->getDungeonLevel() < FIRST_CAVERN_LEVEL - 1) {
    engine->dungeonClimb->travelDown(1);
    engine->log->addMessage("I sink downwards!");
  } else {
    engine->log->addMessage("I get a sinking feeling, but nothing happens.");
  }
  setRealDefinitionNames(engine, false);
}

void ScrollOfTeleportation::specificRead(Engine* const engine) {
  engine->player->teleport(false);
  setRealDefinitionNames(engine, false);
}

void ScrollOfStatusOnAllVisibleMonsters::specificRead(Engine* const engine) {
  engine->player->getSpotedEnemies();
  const vector<Actor*>& actors = engine->player->spotedEnemies;

  if(actors.empty() == false) {
    vector<coord> actorPositions;

    for(unsigned int i = 0; i < actors.size(); i++) {
      actorPositions.push_back(actors.at(i)->pos);
    }

    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(actorPositions, clrMagenta,
        BLAST_ANIMATION_DELAY_FACTOR, engine);

    StatusEffect* const effect = getStatusEffect(engine);

    for(unsigned int i = 0; i < actors.size(); i++) {
      actors.at(i)->getStatusEffectsHandler()->tryAddEffect(effect->copy());
    }

    delete effect;

    setRealDefinitionNames(engine, false);

  } else {
    failedToLearnRealName(engine);
  }
}

StatusEffect* ScrollOfEnfeebleEnemies::getStatusEffect(Engine* const engine) {
  const int RND = engine->dice.getInRange(1, 4);
  switch(RND) {
    case 1: {return new StatusConfused(engine);}  break;
    case 2: {return new StatusParalyzed(engine);} break;
    case 3: {return new StatusSlowed(engine);}    break;
    case 4: {return new StatusBlind(engine);}     break;
  }
  return NULL;
}

void ScrollOfDetectItems::specificRead(Engine* const engine) {
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      Item* item = engine->map->items[x][y];
      if(item != NULL) {
        engine->map->playerVision[x][y] = true;
        engine->map->explored[x][y] = true;
      }
    }
  }
  engine->renderer->drawMapAndInterface();
  engine->player->updateFov();
  engine->renderer->drawMapAndInterface();

  engine->log->addMessage("All items are revealed to me.");
  setRealDefinitionNames(engine, false);
}

void ScrollOfBlessing::specificRead(Engine* const engine) {
  engine->player->getStatusEffectsHandler()->tryAddEffect(new StatusBlessed(engine));
  setRealDefinitionNames(engine, false);
}

void ScrollOfDetectTraps::specificRead(Engine* const engine) {
  bool somethingRevealed = false;

  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      FeatureStatic* const f = engine->map->featuresStatic[x][y];
      if(f->getId() == feature_trap) {
        Trap* const trap = dynamic_cast<Trap*>(f);
        trap->reveal(false);
        somethingRevealed = true;
      }
    }
  }

  if(somethingRevealed) {
    engine->log->addMessage("All traps are revealed to me.");
    setRealDefinitionNames(engine, false);
  } else {
    failedToLearnRealName(engine);
  }
}

void ScrollOfIdentify::specificRead(Engine* const engine) {
  Inventory* const inv = engine->player->getInventory();

  vector<Item*> itemIdentifyCandidates;

  vector<InventorySlot>* slots = inv->getSlots();
  for(unsigned int i = 0; i < slots->size(); i++) {
    Item* const item = slots->at(i).item;
    if(item != NULL) {
      const ItemDefinition& d = item->getDef();
      if(d.isIdentified == false) {
        itemIdentifyCandidates.push_back(item);
      }
    }
  }
  vector<Item*>* backpack = inv->getGeneral();
  for(unsigned int i = 0; i < backpack->size(); i++) {
    Item* const item = backpack->at(i);
    if(item != this) {
      const ItemDefinition& d = item->getDef();
      if(d.isIdentified == false) {
        itemIdentifyCandidates.push_back(item);
      }
    }
  }

  const unsigned int NR_ELEMENTS = itemIdentifyCandidates.size();
  if(NR_ELEMENTS == 0) {
    failedToLearnRealName(engine);
    return;
  } else {
    Item* const item = itemIdentifyCandidates.at(engine->dice.getInRange(0, NR_ELEMENTS - 1));

    const string itemNameBefore = engine->itemData->getItemRef(item, itemRef_a, true);

    const ItemDefinition& d = item->getDef();
    if(d.isScroll) {
      Scroll* const scroll = dynamic_cast<Scroll*>(item);
      scroll->setRealDefinitionNames(engine, true);
    } else if(d.isQuaffable) {
      Potion* const potion = dynamic_cast<Potion*>(item);
      potion->setRealDefinitionNames(engine, true);
    } else if(d.isDevice) {
      Device* const device = dynamic_cast<Device*>(item);
      device->identify(true);
    } else {
      tracer << "[WARNING] Scroll of identify was unable to identify item with name \"" +
      itemNameBefore + "\", in ScrollOfIdentify::specificRead()" << endl;
    }

    const string itemNameAfter = engine->itemData->getItemRef(item, itemRef_a, true);

    engine->log->addMessage("I gain intuitions about " + itemNameBefore + "...");
    engine->log->addMessage("It is identified as " + itemNameAfter + "!");

    setRealDefinitionNames(engine, false);
  }
}

void ScrollOfClairvoyance::specificRead(Engine* const engine) {
  engine->player->getStatusEffectsHandler()->tryAddEffect(new StatusClairvoyant(engine), true, false);
  setRealDefinitionNames(engine, false);
}

void ScrollOfOpening::specificRead(Engine* const engine) {
  bool isSomethingOpened;

  bool playerVisionBefore[MAP_X_CELLS][MAP_Y_CELLS];
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      playerVisionBefore[x][y] = engine->map->playerVision[x][y];
    }
  }

  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(playerVisionBefore[x][y]) {
        if(engine->map->featuresStatic[x][y]->openFeature()) {
          isSomethingOpened = true;
        }
      }
    }
  }

  if(isSomethingOpened) {
    engine->log->addMessage("Everything around me was opened.");
    engine->player->updateFov();
    engine->renderer->drawMapAndInterface();
    setRealDefinitionNames(engine, false);
  } else {
    failedToLearnRealName(engine);
  }
}


void ScrollOfAzathothsBlast::specificRead(Engine* const engine) {
  engine->player->getSpotedEnemies();
  const vector<Actor*>& actors = engine->player->spotedEnemies;

  if(actors.empty() == false) {
    vector<coord> actorPositions;

    for(unsigned int i = 0; i < actors.size(); i++) {
      actorPositions.push_back(actors.at(i)->pos);
    }

    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(actorPositions, clrRedLight,
        BLAST_ANIMATION_DELAY_FACTOR, engine);

    for(unsigned int i = 0; i < actors.size(); i++) {
      const string monsterName = actors.at(i)->getNameThe();
      engine->log->addMessage(monsterName + " is struck by a roaring blast!", clrMessageGood);
      actors.at(i)->getStatusEffectsHandler()->tryAddEffect(new StatusParalyzed(1), false, false);
      actors.at(i)->hit(engine->dice(1, 8), damageType_physical);
    }

    setRealDefinitionNames(engine, false);
  } else {
    failedToLearnRealName(engine);
  }
}

void ThaumaturgicAlteration::specificRead(Engine* const engine) {
  vector<MthPowerAction_t> possibleActions;
  getPossibleActions(possibleActions, engine);

  if(possibleActions.empty()) {
    engine->log->addMessage("I fail to grasp what I want to do.");
  } else {
    vector<string> choiceLabels;
    getChoiceLabelsFromPossibleActions(possibleActions, choiceLabels);

    const unsigned int choiceNr =
      engine->popup->showMultiChoiceMessage("I want to...", true, choiceLabels, "");

    doAction(possibleActions.at(choiceNr), engine);
  }
}

void ThaumaturgicAlteration::doAction(const MthPowerAction_t action, Engine* const engine) const {
  switch(action) {
    case mthPowerAction_slayMonsters: {
      engine->player->getSpotedEnemies();
      const vector<Actor*>& actors = engine->player->spotedEnemies;

      vector<coord> actorPositions;
      for(unsigned int i = 0; i < actors.size(); i++) {
        actorPositions.push_back(actors.at(i)->pos);
      }

      engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
        actorPositions, clrYellow,
        BLAST_ANIMATION_DELAY_FACTOR, engine);

      for(unsigned int i = 0; i < actors.size(); i++) {
        const string monsterName = actors.at(i)->getNameThe();
        engine->log->addMessage(monsterName + " is crushed by an unseen force!", clrMessageGood);
        actors.at(i)->hit(25, damageType_physical);
      }

      engine->renderer->drawMapAndInterface(true);
    } break;

    case mthPowerAction_heal: {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
      engine->player->getStatusEffectsHandler()->endEffect(statusDiseased, visionBlockers);
      engine->player->restoreHP(999);
    } break;

    case mthPowerAction_findStairs: {
      tracer << "ThaumaturgicAlteration: Find stairs" << endl;
      for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
        for(int x = 1; x < MAP_X_CELLS - 1; x++) {
          if(engine->map->featuresStatic[x][y]->getId() == feature_stairsDown) {
            for(int dy = -1; dy <= 1; dy++) {
              for(int dx = -1; dx <= 1; dx++) {
                engine->map->playerVision[x + dx][y + dy] = true;
                engine->map->explored[x + dx][y + dy] = true;
              }
            }
          }
        }
      }
      engine->log->addMessage("The way forward is revealed!");
      engine->renderer->drawMapAndInterface(true);
      engine->player->updateFov();
      engine->renderer->drawMapAndInterface(true);
    } break;

    case mthPowerAction_sorcery: {
      engine->log->addMessage("My magic is restored!");
      const unsigned int NR_OF_SCROLLS = engine->playerPowersHandler->getNrOfScrolls();
      for(unsigned int i = 0; i < NR_OF_SCROLLS; i++) {
        Scroll* const scroll =  engine->playerPowersHandler->getScrollAt(i);
        const ItemDefinition& d = scroll->getDef();
        if(d.isScrollLearnable && d.isScrollLearned && d.id != item_thaumaturgicAlteration) {
          scroll->setCastFromMemoryCurrentBaseChance(CAST_FROM_MEMORY_CHANCE_LIM);
        }
      }
    } break;

    case mthPowerAction_mendArmor: {
      engine->log->addMessage("My armor is whole!");
      Armor* const armor =
        dynamic_cast<Armor*>(engine->player->getInventory()->getItemInSlot(slot_armorBody));
      armor->setMaxDurability();
      engine->renderer->drawMapAndInterface(true);
    } break;

    case mthPowerAction_improveWeapon: {
      engine->log->addMessage("My weapon is deadlier!");
      Weapon* const weapon =
        dynamic_cast<Weapon*>(engine->player->getInventory()->getItemInSlot(slot_wielded));
      weapon->meleeDmgPlus++;
    } break;

    case mthPowerAction_purgeEffects: {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers, 9999);

      StatusEffectsHandler* const statusHandler = engine->player->getStatusEffectsHandler();
      statusHandler->endEffectsOfAbility(ability_resistStatusBody, visionBlockers);
      statusHandler->endEffectsOfAbility(ability_resistStatusMind, visionBlockers);

//      engine->player->getStatusEffectsHandler()->tryAddEffect(
//        new StatusPerfectFortitude(engine));
//      engine->player->restoreShock(999, false);
    } break;
  }
}

void ThaumaturgicAlteration::getPossibleActions(
  vector<MthPowerAction_t>& possibleActions, Engine* const engine) const {

  possibleActions.resize(0);

  engine->player->getSpotedEnemies();
  const vector<Actor*>& actors = engine->player->spotedEnemies;

  const int MTH = engine->player->getMth();

  if(MTH >= 35) {
    if(actors.empty() == false) {
      possibleActions.push_back(mthPowerAction_slayMonsters);
    }
  }

  for(int y = 1; y < MAP_Y_CELLS; y++) {
    for(int x = 1; x < MAP_X_CELLS; x++) {
      if(engine->map->featuresStatic[x][y]->getId() == feature_stairsDown) {
        if(engine->map->explored[x][y] == false) {
          possibleActions.push_back(mthPowerAction_findStairs);
        }
      }
    }
  }

  bool canAnySpellBeRestored = false;
  for(unsigned int i = 0; i < endOfItemIds; i++) {
    const ItemDefinition* d = engine->itemData->itemDefinitions[i];
    if(d->isScroll && d->isScrollLearned &&
        d->castFromMemoryCurrentBaseChance < CAST_FROM_MEMORY_CHANCE_LIM &&
        d->id != item_thaumaturgicAlteration) {
      canAnySpellBeRestored = true;
    }
  }
  if(canAnySpellBeRestored) {
    possibleActions.push_back(mthPowerAction_sorcery);
  }

  if(engine->player->getHp() < engine->player->getHpMax(true)) {
    possibleActions.push_back(mthPowerAction_heal);
  }

  Item* item = engine->player->getInventory()->getItemInSlot(slot_wielded);
  if(item != NULL) {
    const ItemDefinition& d = item->getDef();
    if(d.isMeleeWeapon && d.isRangedWeapon == false) {
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      if(weapon->meleeDmgPlus < 3) {
        possibleActions.push_back(mthPowerAction_improveWeapon);
      }
    }
  }

  item = engine->player->getInventory()->getItemInSlot(slot_armorBody);
  if(item != NULL) {
    Armor* const armor = dynamic_cast<Armor*>(item);
    if(armor->getDurability() < 100) {
      possibleActions.push_back(mthPowerAction_mendArmor);
    }
  }

  possibleActions.push_back(mthPowerAction_purgeEffects);
}

void ThaumaturgicAlteration::getChoiceLabelsFromPossibleActions(
  const vector<MthPowerAction_t>& possibleActions, vector<string>& labels) const {

  labels.resize(0);

  for(unsigned int i = 0; i < possibleActions.size(); i++) {
    switch(possibleActions.at(i)) {
      case mthPowerAction_findStairs: {
        labels.push_back("Reveal the way forward!");
      } break;
      case mthPowerAction_sorcery: {
        labels.push_back("Restore my magic!");
      } break;
      case mthPowerAction_heal: {
        labels.push_back("Heal myself!");
      } break;
      case mthPowerAction_improveWeapon: {
        labels.push_back("Make my weapon deadlier!");
      } break;
      case mthPowerAction_mendArmor: {
        labels.push_back("Mend my armor!");
      } break;
      case mthPowerAction_slayMonsters: {
        labels.push_back("Slay my enemies!");
      } break;
      case mthPowerAction_purgeEffects: {
        labels.push_back("Purge all harmful effects!");
      } break;
    }
  }
}

//void ScrollOfVoidChain::specificRead(Engine* const engine) {
//  setRealDefinitionNames(engine, false);
//  if(engine->player->getStatusEffectsHandler()->allowAct()) {
//    engine->marker->place(markerTask_spellVoidChain);
//  } else {
//    engine->log->addMessage("My spell is disrupted.");
//  }
//}

//void ScrollOfVoidChain::castAt(const coord& pos, Engine* const engine) {
//  const coord playerPos = engine->player->pos;
//  const vector<coord> projectilePath =
//    engine->mapTests->getLine(playerPos.x, playerPos.y, pos.x, pos.y,
//                              true, FOV_STANDARD_RADI_INT);
//}

//void ScrollOfIbnGhazisPowder::specificRead(Engine* const engine) {
//  setRealDefinitionNames(engine, false);
//  if(engine->player->getStatusEffectsHandler()->allowAct()) {
//    engine->query->direction();
//  } else {
//    engine->log->addMessage("My spell is disrupted.");
//  }
//}

void ScrollNameHandler::setFalseScrollName(ItemDefinition* d) {
  const unsigned int NR_NAMES = m_falseNames.size();

  const unsigned int ELEMENT = static_cast<unsigned int>(eng->dice(1, NR_NAMES) - 1);

  const string TITLE = "\"" + m_falseNames.at(ELEMENT) + "\"";

  m_falseNames.erase(m_falseNames.begin() + ELEMENT);

  d->name.name = "Manuscript titled " + TITLE;
  d->name.name_plural = "Manuscripts titled " + TITLE;
  d->name.name_a = "a Manuscript titled " + TITLE;
}

void ScrollNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    if(eng->itemData->itemDefinitions[i]->isReadable == true) {
      lines.push_back(eng->itemData->itemDefinitions[i]->name.name);
      lines.push_back(eng->itemData->itemDefinitions[i]->name.name_plural);
      lines.push_back(eng->itemData->itemDefinitions[i]->name.name_a);
    }
  }
}

void ScrollNameHandler::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    if(eng->itemData->itemDefinitions[i]->isReadable == true) {
      eng->itemData->itemDefinitions[i]->name.name = lines.front();
      lines.erase(lines.begin());
      eng->itemData->itemDefinitions[i]->name.name_plural = lines.front();
      lines.erase(lines.begin());
      eng->itemData->itemDefinitions[i]->name.name_a = lines.front();
      lines.erase(lines.begin());
    }
  }
}

Scroll::~Scroll() {

}

int Scroll::getChanceToCastFromMemory(Engine* const engine) const {
  (void)engine;
  const int BASE_CHANCE = def_->castFromMemoryCurrentBaseChance;

  int bon = 0;

  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      const coord pos(engine->player->pos + coord(dx, dy));
      if(engine->map->featuresStatic[pos.x][pos.y]->getId() == feature_altar) {
        bon += 20;
        break;
      }
    }
  }

  bon += engine->playerBonusHandler->isBonusPicked(playerBonus_warlock)       ? 20 : 0;

  bon += engine->player->getStatusEffectsHandler()->hasEffect(statusBlessed)  ? 10 : 0;
  bon += engine->player->getStatusEffectsHandler()->hasEffect(statusBlind)    ? 10 : 0;

  bon += engine->player->getStatusEffectsHandler()->hasEffect(statusCursed)   ? -25 : 0;
  bon += engine->player->getStatusEffectsHandler()->hasEffect(statusConfused) ? -25 : 0;

  bon += engine->player->getMth() / CAST_FROM_MEMORY_MTH_BON_DIV;

  return max(0, min(CAST_FROM_MEMORY_CHANCE_LIM, BASE_CHANCE + bon));
}

void Scroll::setCastFromMemoryCurrentBaseChance(const int VAL) {
  def_->castFromMemoryCurrentBaseChance = max(0, min(CAST_FROM_MEMORY_CHANCE_LIM, VAL));
}

void Scroll::setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY) {
  if(def_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME = "Manuscript of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Manuscripts of " + REAL_TYPE_NAME;
    const string REAL_NAME_A = "a Manuscript of " + REAL_TYPE_NAME;

    def_->name.name = REAL_NAME;
    def_->name.name_plural = REAL_NAME_PLURAL;
    def_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      engine->log->addMessage("It was " + def_->name.name_a + ".");
      engine->renderer->drawMapAndInterface();
    }

    def_->isIdentified = true;
  }
}

void Scroll::tryMemorizeIfLearnable(Engine* const engine) {
  if(def_->isScrollLearned == false && def_->isScrollLearnable) {
    const int CHANCE_TO_LEARN = 90;
    if(engine->dice.percentile() < CHANCE_TO_LEARN) {
      engine->log->addMessage("I learn to cast this incantation by heart!");
      def_->isScrollLearned = true;
    } else {
      engine->log->addMessage("I failed to memorize the incantation.");
    }
  }
}

bool Scroll::read(const bool IS_FROM_MEMORY, Engine* const engine) {
  engine->renderer->drawMapAndInterface();

  if(IS_FROM_MEMORY) {
    return tryReadFromMemory(engine);
  } else {
    return tryReadFromScroll(engine);
  }
}

bool Scroll::tryReadFromMemory(Engine* const engine) {
  const AbilityRollResult_t rollResult = engine->abilityRoll->roll(getChanceToCastFromMemory(engine));
  if(rollResult >= successSmall) {
    engine->log->addMessage("I cast " + getRealTypeName() + "...");
    def_->castFromMemoryCurrentBaseChance = 0;
    specificRead(engine);
  } else {
    engine->log->addMessage("I miscast it.");
    const int CHANCE_DECR_WHEN_FAIL = 2;
    if(def_->castFromMemoryCurrentBaseChance >= CHANCE_DECR_WHEN_FAIL) {
      def_->castFromMemoryCurrentBaseChance -= CHANCE_DECR_WHEN_FAIL;
    }
    engine->player->getStatusEffectsHandler()->tryAddEffect(new StatusWeak(engine));
    if(engine->dice.coinToss()) {
      engine->log->addMessage("I feel a sharp pain in my head!", clrMessageBad);
    } else {
      engine->log->addMessage("It feels like a dagger piercing my skull!", clrMessageBad);
    }
    engine->player->getStatusEffectsHandler()->tryAddEffect(new StatusParalyzed(engine), false, false);
    engine->player->hit(engine->dice(1, 6), damageType_pure);
  }
  if(engine->player->deadState == actorDeadState_alive) {
    engine->player->incrShock(SHOCK_TAKEN_FROM_CASTING_SPELLS);
    engine->gameTime->letNextAct();
  }
  return true;
}

bool Scroll::tryReadFromScroll(Engine* const engine) {
  if(engine->player->getStatusEffectsHandler()->allowSee() == false) {
    engine->log->addMessage("I cannot read while blind.");
    return false;
  }

//  if(engine->playerBonusHandler->isBonusPicked(playerBonus_learned) == false) {
//    engine->log->addMessage("I cannot yet comprehend this.");
//    return false;
//  }

  const bool IS_IDENTIFIED_BEFORE_READING = def_->isIdentified;
  const bool IS_SCROLL_LEARNED_BEFORE_READING = def_->isScrollLearned;

  if(IS_IDENTIFIED_BEFORE_READING) {
    engine->log->addMessage("I read a scroll of " + getRealTypeName() + "...");
    specificRead(engine);
    tryMemorizeIfLearnable(engine);
    engine->player->incrShock(SHOCK_TAKEN_FROM_CASTING_SPELLS);
  } else {
    engine->log->addMessage("I recite forbidden incantations...");
    def_->isTried = true;
    specificRead(engine);
    engine->player->incrShock(SHOCK_TAKEN_FROM_CASTING_SPELLS);
  }

  if(IS_SCROLL_LEARNED_BEFORE_READING) {
    setCastFromMemoryCurrentBaseChance(def_->castFromMemoryCurrentBaseChance + 20);
  }

  engine->gameTime->letNextAct();
  return true;
}

void Scroll::failedToLearnRealName(Engine* const engine, const string overrideFailString) {
  if(def_->isIdentified == false) {
    if(overrideFailString != "") {
      engine->log->addMessage(overrideFailString);
    } else {
      engine->log->addMessage("Was that supposed to do something?");
    }
  }
}

