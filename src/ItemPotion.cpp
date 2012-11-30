#include "ItemPotion.h"

#include "Engine.h"
#include "StatusEffects.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ActorMonster.h"

//void PotionOfClairvoyance::specificQuaff(Actor* const actor, Engine* const engine) {
//
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  engine->mapTests->makeVisionBlockerArray(actor->pos, blockers);
//
//  if(actor == engine->player) {
//    engine->log->addMessage("A vision comes to me, the area is revealed.");
//
//    for(int y = MAP_Y_CELLS - 1; y >= 0; y--) {
//      for(int x = MAP_X_CELLS - 1; x >= 0; x--) {
//        if(blockers[x][y] == false) {
//          engine->map->playerVision[x][y] = true;
//          engine->map->explored[x][y] = true;
//        }
//        //TODO reimplement
//        //				Door* door = engine->map->doors[x][y];
//        //				if(door != NULL) {
//        //					if(door->isSecret() == true) {
//        //						door->reveal(false, engine);
//        //					}
//        //				}
//      }
//    }
//
//    engine->renderer->drawMapAndInterface();
//
//    setRealDefinitionNames(engine, false);
//  }
//}

void PotionOfHealing::specificQuaff(Actor* const actor, Engine* const engine) {
  //Attempt to heal the actor. If no hp was healed (already at full hp), boost the hp instead.
  if(actor->restoreHP(engine->dice(2, 6) + 12) == false) {
    actor->changeMaxHP(1, true);
  }

  //End disease
  actor->getStatusEffectsHandler()->endEffect(statusDiseased);

  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfHealing::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfBlindness::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBlind(8 + engine->dice(1, 8)));
  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
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
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusParalyzed(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
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
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusDiseased(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfFear::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusTerrified(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfFear::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfConfusion::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusConfused(engine));
  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfConfusion::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfCorruption::specificQuaff(Actor* const actor, Engine* const engine) {
  const int CHANGE = -(engine->dice(1, 2));

  actor->changeMaxHP(CHANGE, true);

  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfCorruption::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  if(actor != NULL) {
    specificQuaff(actor, engine);
  } else {
    engine->map->switchToDestroyedFeatAt(pos);

    if(engine->map->playerVision[pos.x][pos.y] == true) {
      setRealDefinitionNames(engine, false);
    }
  }
}

void PotionOfTheCobra::specificQuaff(Actor* const actor, Engine* const engine) {
  const int TURNS = engine->dice(3, 8) + 24;
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusPerfectAim(TURNS));
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusPerfectReflexes(TURNS));
  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfTheCobra::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfStealth::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusPerfectStealth(100 + engine->dice(8, 8)));
  for(unsigned int i = 0; i < engine->gameTime->getLoopSize(); i++) {
    Actor* otherActor = engine->gameTime->getActorAt(i);
    if(otherActor != engine->player) {
      dynamic_cast<Monster*>(otherActor)->playerAwarenessCounter = 0;
    }
  }
  setRealDefinitionNames(engine, false);
}

void PotionOfFortitude::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusPerfectFortitude(24 + engine->dice(3, 8)));

  actor->getStatusEffectsHandler()->endEffectsOfAbility(ability_resistStatusMind);

  bool isPhobiasCured = false;
  for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
    if(engine->player->insanityPhobias[i] == true) {
      engine->player->insanityPhobias[i] = false;
      isPhobiasCured = true;
    }
  }
  if(isPhobiasCured) {
    engine->log->addMessage("All my phobias are cured!");
  }

  bool isCompulsionsCured = false;
  for(unsigned int i = 0; i < endOfInsanityCompulsions; i++) {
    if(engine->player->insanityCompulsions[i] == true) {
      engine->player->insanityCompulsions[i] = false;
      isCompulsionsCured = true;
    }
  }
  if(isCompulsionsCured) {
    engine->log->addMessage("All my compulsions are cured!");
  }

  const bool IS_SHOCK_RESTORED = engine->player->insanityShort > 0;
  if(IS_SHOCK_RESTORED) {
    engine->player->insanityShort = 0;
    engine->log->addMessage("I feel at ease. ");
  }

  setRealDefinitionNames(engine, false);
}

void PotionOfFortitude::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor, engine);
  }
}

void PotionOfToughness::specificQuaff(Actor* const actor, Engine* const engine) {
  actor->getStatusEffectsHandler()->attemptAddEffect(new StatusPerfectToughness(24 + engine->dice(3, 8)));

  actor->getStatusEffectsHandler()->endEffectsOfAbility(ability_resistStatusBody);

  if(engine->player->checkIfSeeActor(*actor, NULL) == true) {
    setRealDefinitionNames(engine, false);
  }
}

void PotionOfToughness::specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
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
  for(unsigned int i = 1; i < endOfItemDevNames; i++) {
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
  for(unsigned int i = 1; i < endOfItemDevNames; i++) {
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

    engine->log->addMessage("It was a " + REAL_NAME + ".");

    if(IS_SILENT_IDENTIFY == false) {
      engine->player->shock(shockValue_heavy, 0);
    }

    def_->isIdentified = true;
  }
}

void Potion::collide(const coord& pos, Actor* const actor, const ItemDefinition& itemDef, Engine* const engine) {
  if(engine->map->featuresStatic[pos.x][pos.y]->isBottomless() == false || actor != NULL) {
    ItemDefinition* const potionDef = engine->itemData->itemDefinitions[itemDef.devName];

    const bool PLAYER_SEE_CELL = engine->map->playerVision[pos.x][pos.y];

    if(PLAYER_SEE_CELL == true) {
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

    engine->player->shock(shockValue_heavy, 0);

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
