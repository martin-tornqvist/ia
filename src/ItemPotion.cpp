#include "ItemPotion.h"

#include "Engine.h"
#include "StatusEffects.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ActorMonster.h"
#include "PlayerPowersHandler.h"
#include "ItemScroll.h"

void PotionOfHealing::specificQuaff(Actor* const actor, Engine* const engine) {
  //End disease
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  actor->getStatusEffectsHandler()->endEffect(statusDiseased, visionBlockers);

  //Attempt to heal the actor. If no hp was healed (already at full hp), boost the hp instead.
  if(actor->restoreHP(engine->dice(2, 6) + 12) == false) {
    actor->changeMaxHP(1, true);
  }

  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfHealing::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfSorcery::specificQuaff(Actor* const actor, Engine* const engine) {
  (void)actor;
  bool isAnySpellRestored = false;

  const unsigned int NR_OF_SCROLLS = engine->playerPowersHandler->getNrOfScrolls();
  for(unsigned int i = 0; i < NR_OF_SCROLLS; i++) {
    Scroll* const scroll =  engine->playerPowersHandler->getScrollAt(i);
    const ItemDefinition& d = scroll->getDef();
    if(d.isScrollLearnable && d.isScrollLearned) {
      if(d.castFromMemoryCurrentBaseChance < CAST_FROM_MEMORY_CHANCE_LIM) {
        scroll->setCastFromMemoryCurrentBaseChance(CAST_FROM_MEMORY_CHANCE_LIM);
        isAnySpellRestored = true;
      }
    }
  }

  if(isAnySpellRestored) {
    engine->log->addMessage("My magic is restored!");
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfBlindness::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusBlind(8 + engine->dice(1, 8)));
  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfBlindness::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfParalyzation::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusParalyzed(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfParalyzation::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfDisease::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusDiseased(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfConfusion::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusConfused(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfConfusion::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

//void PotionOfCorruption::specificQuaff(Actor* const actor, Engine* const engine) {
//  const int CHANGE = -(engine->dice(1, 2));
//
//  actor->changeMaxHP(CHANGE, true);
//
//  if(engine->player->checkIfSeeActor(*actor, NULL)) {
//    setRealDefinitionNames(engine, false);
//  }
//}
//
//void PotionOfCorruption::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
//  if(actor != NULL) {
//    specificQuaff(actor, engine);
//  } else {
//    engine->map->switchToDestroyedFeatAt(pos);
//
//    if(engine->map->playerVision[pos.x][pos.y]) {
//      setRealDefinitionNames(engine, false);
//    }
//  }
//}

void PotionOfTheCobra::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusPerfectAim(engine));
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusPerfectReflexes(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfTheCobra::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

//void PotionOfStealth::specificQuaff(Actor* const actor, Engine* const engine) {
//  actor->getStatusEffectsHandler()->tryAddEffect(new StatusPerfectStealth(100 + engine->dice(8, 8)));
//  for(unsigned int i = 0; i < engine->gameTime->getLoopSize(); i++) {
//    Actor* otherActor = engine->gameTime->getActorAt(i);
//    if(otherActor != engine->player) {
//      dynamic_cast<Monster*>(otherActor)->playerAwarenessCounter = 0;
//    }
//  }
//  setRealDefinitionNames(engine, false);
//}

void PotionOfFortitude::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusPerfectFortitude(engine));

  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  actor->getStatusEffectsHandler()->endEffectsOfAbility(ability_resistStatusMind, visionBlockers);

  bool isPhobiasCured = false;
  for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
    if(engine->player->insanityPhobias[i]) {
      engine->player->insanityPhobias[i] = false;
      isPhobiasCured = true;
    }
  }
  if(isPhobiasCured) {
    engine->log->addMessage("All my phobias are cured!");
  }

  bool isObsessionsCured = false;
  for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
    if(engine->player->insanityObsessions[i]) {
      engine->player->insanityObsessions[i] = false;
      isObsessionsCured = true;
    }
  }
  if(isObsessionsCured) {
    engine->log->addMessage("All my obsessions are cured!");
  }

  engine->player->restoreShock(999, false);
  engine->log->addMessage("I feel more at ease.");

  setRealDefinitionNames(engine, false);
}

void PotionOfFortitude::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfToughness::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusPerfectToughness(engine));

  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  actor->getStatusEffectsHandler()->endEffectsOfAbility(ability_resistStatusBody, visionBlockers);

  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfToughness::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfPoison::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->tryAddEffect(new StatusPoisoned(engine));

  if(engine->player->checkIfSeeActor(*actor, NULL)) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfPoison::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionNameHandler::setColorAndFalseName(ItemDefinition* d) {
  const unsigned int NR_NAMES = m_falseNames.size();

  const unsigned int ELEMENT = static_cast<unsigned int>(eng->dice(1, NR_NAMES) - 1);

  const string DESCRIPTION = m_falseNames.at(ELEMENT).str;
  const sf::Color clr = m_falseNames.at(ELEMENT).color;

  m_falseNames.erase(m_falseNames.begin() + ELEMENT);

  d->name.name = DESCRIPTION + " potion";
  d->name.name_plural = DESCRIPTION + " potions";
  d->name.name_a = "a " + DESCRIPTION + " potion";
  d->color = clr;
}

void PotionNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isQuaffable) {
      lines.push_back(d->name.name);
      lines.push_back(d->name.name_plural);
      lines.push_back(d->name.name_a);
      lines.push_back(intToString(d->color.r));
      lines.push_back(intToString(d->color.g));
      lines.push_back(intToString(d->color.b));
    }
  }
}

void PotionNameHandler::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isQuaffable) {
      d->name.name = lines.front();
      lines.erase(lines.begin());
      d->name.name_plural = lines.front();
      lines.erase(lines.begin());
      d->name.name_a = lines.front();
      lines.erase(lines.begin());
      d->color.r = stringToInt(lines.front());
      lines.erase(lines.begin());
      d->color.g = stringToInt(lines.front());
      lines.erase(lines.begin());
      d->color.b = stringToInt(lines.front());
      lines.erase(lines.begin());
    }
  }
}

void Potion::setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY) {
  if(def_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME = "Potion of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Potions of " + REAL_TYPE_NAME;
    const string REAL_NAME_A = "a potion of " + REAL_TYPE_NAME;

    def_->name.name = REAL_NAME;
    def_->name.name_plural = REAL_NAME_PLURAL;
    def_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      engine->log->addMessage("It was a " + REAL_NAME + ".");
      engine->player->incrShock(shockValue_heavy);
    }

    def_->isIdentified = true;
  }
}

void Potion::collide(const coord& pos, Actor* const actor, const ItemDefinition& itemDef, Engine* const engine) {
  if(engine->map->featuresStatic[pos.x][pos.y]->isBottomless() == false || actor != NULL) {
    ItemDefinition* const potionDef = engine->itemData->itemDefinitions[itemDef.id];

    const bool PLAYER_SEE_CELL = engine->map->playerVision[pos.x][pos.y];

    if(PLAYER_SEE_CELL) {
      engine->renderer->drawCharacter('*', renderArea_mainScreen, pos.x, pos.y, potionDef->color);

      if(actor != NULL) {
        if(actor->deadState == actorDeadState_alive) {
          engine->log->addMessage("The potion shatters on " + actor->getNameThe() + ".");
        }
      } else {
        engine->log->addMessage("The potion shatters on " + engine->map->featuresStatic[pos.x][pos.y]->getDescription(true) + ".");
      }
    }
    //If the blow from the bottle didn't kill the actor, apply what's inside
    if(actor != NULL) {
      if(actor->deadState == actorDeadState_alive) {
        specificCollide(pos, actor, engine);

        if(actor->deadState == actorDeadState_alive && potionDef->isIdentified == false && PLAYER_SEE_CELL) {
          engine->log->addMessage("It had no apparent effect...");
        }
      }
    }
  }
}

void Potion::quaff(Actor* const actor, Engine* const engine) {
  if(actor == engine->player) {
    def_->isTried = true;

    engine->player->incrShock(shockValue_heavy);

    if(def_->isIdentified) {
      engine->log->addMessage("I drink " + def_->name.name_a + "...");
    } else {
      engine->log->addMessage("I drink an unknown " + def_->name.name + "...");
    }
  }

  specificQuaff(actor, engine);

  if(engine->player->deadState == actorDeadState_alive) {
    engine->gameTime->letNextAct();
  }
}

void Potion::failedToLearnRealName(Engine* const engine, const string overrideFailString) {
  if(def_->isIdentified == false) {
    if(overrideFailString != "") {
      engine->log->addMessage(overrideFailString);
    } else {
      engine->log->addMessage("It doesn't seem to affect me.");
    }
  }
}
