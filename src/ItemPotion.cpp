#include "ItemPotion.h"

#include "Engine.h"
#include "Properties.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ActorMonster.h"
#include "PlayerSpellsHandler.h"
#include "ItemScroll.h"
#include "GameTime.h"
#include "Audio.h"
#include "Renderer.h"

void PotionOfHealing::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->endAppliedPropsByMagicHealing();

  //TODO What about wounds?

  //Attempt to heal the actor. If no hp was healed (already at full hp),
  //boost the hp instead.
  if(actor->restoreHp(999, true) == false) {
    actor->changeMaxHp(1, true);
  }

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfHealing::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfSpirit::specificQuaff(Actor* const actor) {
  //Attempt to restore spirit. If no hp was healed (already at full hp),
  //boost the hp instead.
  if(actor->restoreSpi(eng->dice(2, 6) + 12, true) == false) {
    actor->changeMaxSpi(1, true);
  }

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfSpirit::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

//void PotionOfSorcery::specificQuaff(Actor* const actor, eng* const eng) {
//  (void)actor;
//  bool isAnySpellRestored = false;
//
//  const unsigned int NR_OF_SCROLLS = eng->playerPowersHandler->getNrOfSpells();
//  for(unsigned int i = 0; i < NR_OF_SCROLLS; i++) {
//    Scroll* const scroll =  eng->playerPowersHandler->getScrollAt(i);
//    const ItemDef& d = scroll->getData();
//    if(
//      d.isScrollLearnable &&
//      d.isScrollLearned   &&
//      d.id != item_thaumaturgicAlteration) {
//      if(d.castFromMemoryCurrentBaseChance < CAST_FROM_MEMORY_CHANCE_LIM) {
//        scroll->setCastFromMemoryCurrentBaseChance(CAST_FROM_MEMORY_CHANCE_LIM);
//        isAnySpellRestored = true;
//      }
//    }
//  }
//
//  if(isAnySpellRestored) {
//    eng->log->addMsg("My magic is restored!");
//    identify(false);
//  }
//}

void PotionOfBlindness::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropBlind(eng, propTurnsStandard));
  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfBlindness::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {specificQuaff(actor);}
}

void PotionOfParalyzation::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropParalyzed(eng, propTurnsStandard));
  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfParalyzation::specificCollide(
  const Pos& pos, Actor* const actor) {

  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfDisease::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropDiseased(eng, propTurnsStandard));
  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfConfusion::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropConfused(eng, propTurnsStandard));
  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfConfusion::specificCollide(
  const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

//void PotionOfCorruption::specificQuaff(Actor* const actor, eng* const eng) {
//  const int CHANGE = -(eng->dice(1, 2));
//
//  actor->changeMaxHP(CHANGE, true);
//
//  if(eng->player->checkIfSeeActor(*actor, NULL)) {
//    identify(false);
//  }
//}
//
//void PotionOfCorruption::specificCollide(const Pos& pos, Actor* const actor, eng* const eng) {
//  if(actor != NULL) {
//    specificQuaff(actor);
//  } else {
//    eng->map->switchToDestroyedFeatAt(pos);
//
//    if(eng->map->playerVision[pos.x][pos.y]) {
//      identify(false);
//    }
//  }
//}

void PotionOfFrenzy::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropFrenzied(eng, propTurnsStandard));
  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfFrenzy::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfFortitude::specificQuaff(Actor* const actor) {
  PropHandler* const propHandler = actor->getPropHandler();

  PropRFear*      const rFear   = new PropRFear(eng, propTurnsStandard);
  PropRConfusion* const rConf   = new PropRConfusion(
    eng, propTurnsSpecified, rFear->turnsLeft_);
  PropRSleep*     const rSleep  = new PropRSleep(
    eng, propTurnsSpecified, rFear->turnsLeft_);

  propHandler->tryApplyProp(rFear);
  propHandler->tryApplyProp(rConf);
  propHandler->tryApplyProp(rSleep);

  if(actor == eng->player) {
    bool isPhobiasCured = false;
    for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
      if(eng->player->insanityPhobias[i]) {
        eng->player->insanityPhobias[i] = false;
        isPhobiasCured = true;
      }
    }
    if(isPhobiasCured) {
      eng->log->addMsg("All my phobias are cured!");
    }

    bool isObsessionsCured = false;
    for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
      if(eng->player->insanityObsessions[i]) {
        eng->player->insanityObsessions[i] = false;
        isObsessionsCured = true;
      }
    }
    if(isObsessionsCured) {
      eng->log->addMsg("All my obsessions are cured!");
    }

    eng->player->restoreShock(999, false);
    eng->log->addMsg("I feel more at ease.");
  }

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfFortitude::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

//void PotionOfToughness::specificQuaff(Actor* const actor, eng* const eng) {
//  actor->getPropHandler()->tryApplyProp(new StatusPerfectToughness(eng));
//
//  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
//  actor->getPropHandler()->endEffectsOfAbility(ability_resistStatusBody, visionBlockers);
//
//  if(eng->player->checkIfSeeActor(*actor, NULL)) {
//    identify(false);
//  }
//}
//
//void PotionOfToughness::specificCollide(const Pos& pos, Actor* const actor, eng* const eng) {
//  (void)pos;
//  if(actor != NULL) {
//    specificQuaff(actor);
//  }
//}

void PotionOfPoison::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropPoisoned(eng, propTurnsStandard));

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfPoison::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfRFire::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropRFire(eng, propTurnsStandard));

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRFire::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

//void PotionOfRCold::specificQuaff(Actor* const actor) {
//  actor->getPropHandler()->tryApplyProp(
//    new PropRCold(eng, propTurnsStandard));
//
//  if(eng->player->checkIfSeeActor(*actor, NULL)) {
//    identify(false);
//  }
//}
//
//void PotionOfRCold::specificCollide(const Pos& pos, Actor* const actor) {
//  (void)pos;
//  if(actor != NULL) {
//    specificQuaff(actor);
//  }
//}

void PotionOfAntidote::specificQuaff(Actor* const actor) {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(actor->pos, visionBlockers, 999);
  const bool IS_POISON_ENDED =
    actor->getPropHandler()->endAppliedProp(propPoisoned, visionBlockers);

  if(IS_POISON_ENDED && eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfAntidote::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfRElec::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropRElec(eng, propTurnsStandard));

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRElec::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfRAcid::specificQuaff(Actor* const actor) {
  actor->getPropHandler()->tryApplyProp(
    new PropRAcid(eng, propTurnsStandard));

  if(eng->player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRAcid::specificCollide(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {
    specificQuaff(actor);
  }
}

void PotionOfKnowledge::specificQuaff(Actor* const actor) {
  (void)actor;
  eng->log->addMsg("I feel more insightful about the mystic powers!");
  eng->player->incrMth(4);
  identify(false);
}

void PotionNameHandler::setColorAndFalseName(ItemData* d) {
  const unsigned int NR_NAMES = m_falseNames.size();

  const unsigned int ELEMENT = (unsigned int)(eng->dice(1, NR_NAMES) - 1);

  const string DESCRIPTION = m_falseNames.at(ELEMENT).str;
  const SDL_Color clr = m_falseNames.at(ELEMENT).clr;

  m_falseNames.erase(m_falseNames.begin() + ELEMENT);

  d->name.name = DESCRIPTION + " potion";
  d->name.name_plural = DESCRIPTION + " potions";
  d->name.name_a = "a " + DESCRIPTION + " potion";
  d->color = clr;
}

void PotionNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    ItemData* const d = eng->itemDataHandler->dataList[i];
    if(d->isPotion) {
      lines.push_back(d->name.name);
      lines.push_back(d->name.name_plural);
      lines.push_back(d->name.name_a);
      lines.push_back(toString(d->color.r));
      lines.push_back(toString(d->color.g));
      lines.push_back(toString(d->color.b));
    }
  }
}

void PotionNameHandler::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    ItemData* const d = eng->itemDataHandler->dataList[i];
    if(d->isPotion) {
      d->name.name = lines.front();
      lines.erase(lines.begin());
      d->name.name_plural = lines.front();
      lines.erase(lines.begin());
      d->name.name_a = lines.front();
      lines.erase(lines.begin());
      d->color.r = toInt(lines.front());
      lines.erase(lines.begin());
      d->color.g = toInt(lines.front());
      lines.erase(lines.begin());
      d->color.b = toInt(lines.front());
      lines.erase(lines.begin());
    }
  }
}

void Potion::identify(const bool IS_SILENT_IDENTIFY) {
  if(data_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME = "Potion of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Potions of " + REAL_TYPE_NAME;
    const string REAL_NAME_A = "a potion of " + REAL_TYPE_NAME;

    data_->name.name = REAL_NAME;
    data_->name.name_plural = REAL_NAME_PLURAL;
    data_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      eng->log->addMsg("It was a " + REAL_NAME + ".");
      eng->player->incrShock(shockValue_heavy);
    }

    data_->isIdentified = true;
  }
}

void Potion::collide(const Pos& pos, Actor* const actor) {
  if(eng->map->featuresStatic[pos.x][pos.y]->isBottomless() == false ||
      actor != NULL) {
//    ItemData* const potData =
//      eng->itemDataHandler->dataList[d.id];

    const bool PLAYER_SEE_CELL = eng->map->playerVision[pos.x][pos.y];

    if(PLAYER_SEE_CELL) {
      // TODO Use standard animation
      eng->renderer->drawGlyph('*', panel_map, pos, data_->color);

      if(actor != NULL) {
        if(actor->deadState == actorDeadState_alive) {
          eng->log->addMsg(
            "The potion shatters on " +
            actor->getNameThe() + ".");
        }
      } else {
        Feature* const f = eng->map->featuresStatic[pos.x][pos.y];
        eng->log->addMsg(
          "The potion shatters on " + f->getDescription(true) + ".");
      }
    }
    //If the blow from the bottle didn't kill the actor, apply what's inside
    if(actor != NULL) {
      if(actor->deadState == actorDeadState_alive) {
        specificCollide(pos, actor);
        if(
          actor->deadState == actorDeadState_alive &&
          data_->isIdentified == false && PLAYER_SEE_CELL) {
          eng->log->addMsg("It had no apparent effect...");
        }
      }
    }
  }
}

void Potion::quaff(Actor* const actor) {
  if(actor == eng->player) {
    data_->isTried = true;

    eng->audio->play(sfxPotionQuaff);

    if(data_->isIdentified) {
      eng->log->addMsg("I drink " + data_->name.name_a + "...");
    } else {
      eng->log->addMsg("I drink an unknown " + data_->name.name + "...");
    }

    eng->player->incrShock(shockValue_heavy);

  }

  specificQuaff(actor);

  if(eng->player->deadState == actorDeadState_alive) {
    eng->gameTime->endTurnOfCurrentActor();
  }
}

void Potion::failedToLearnRealName(const string overrideFailString) {
  if(data_->isIdentified == false) {
    if(overrideFailString != "") {
      eng->log->addMsg(overrideFailString);
    } else {
      eng->log->addMsg("It doesn't seem to affect me.");
    }
  }
}
