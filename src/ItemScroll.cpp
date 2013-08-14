#include "ItemScroll.h"

#include "Engine.h"

#include "ActorPlayer.h"
#include "Map.h"
#include "PlayerBonuses.h"
#include "Log.h"
#include "Inventory.h"
#include "PlayerSpellsHandler.h"

const string Scroll::getRealTypeName() {
  switch(def_->spellCastFromScroll) {
    case spell_azathothsBlast:  return "Azathoths Blast";         break;
    case spell_enfeeble:        return "Enfeeblement";            break;
    case spell_teleport:        return "Teleportation";           break;
    case spell_bless:           return "Blessing";                break;
    case spell_mayhem:          return "Mayhem";                  break;
    case spell_pestilence:      return "Pestilence";              break;
    case spell_descent:         return "Descent";                 break;
    case spell_detectItems:     return "Detect Items";            break;
    case spell_detectTraps:     return "Detect Traps";            break;
    case spell_identify:        return "Identify";                break;
    case spell_clairvoyance:    return "Clairvoyance";            break;
    case spell_opening:         return "Opening";                 break;
    case spell_mthPower:        return "Thaumaturgic Alteration"; break;
    case spell_disease:         return "";                        break;
    case spell_summonRandom:    return "";                        break;
    case spell_healSelf:        return "";                        break;
    case spell_knockBack:       return "";                        break;
    case endOfSpells: {} break;
  }
  tracer << "[WARNING] No spell found for scroll, ";
  tracer << "in Scroll::getRealTypeName()" << endl;
  return "";
}

Spell* Scroll::getSpell(Engine* const engine) {
  return engine->spellHandler->getSpellFromId(def_->spellCastFromScroll);
}

void Scroll::identify(const bool IS_SILENT_IDENTIFY,
                      Engine* const engine) {
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

void Scroll::tryLearn(Engine* const engine) {
  Spell* const spell = getSpell(engine);
  if(
    spell->isLearnableForPlayer() &&
    engine->playerSpellsHandler->isSpellLearned(spell->getId()) == false) {
    engine->log->addMessage("I learn to cast this incantation by heart!");
    engine->playerSpellsHandler->learnSpellIfNotKnown(spell);
  } else {
    delete spell;
  }
}

bool Scroll::read(Engine* const engine) {
  engine->renderer->drawMapAndInterface();

  if(engine->player->getStatusHandler()->allowSee() == false) {
    engine->log->addMessage("I cannot read while blind.");
    return false;
  }

  Spell* const spell = getSpell(engine);

  if(def_->isIdentified) {
    engine->log->addMessage(
      "I read a scroll of " + getRealTypeName() + "...");
    spell->cast(engine->player, engine);
    tryLearn(engine);
  } else {
    engine->log->addMessage("I recite forbidden incantations...");
    def_->isTried = true;
    if(spell->cast(engine->player, engine).IS_CAST_IDENTIFYING) {
      identify(false, engine);
    } else {
      failedToLearnRealName(engine);
    }
  }
  delete spell;

  engine->gameTime->endTurnOfCurrentActor();
  return true;
}

void Scroll::failedToLearnRealName(Engine* const engine,
                                   const string overrideFailString) {
  if(def_->isIdentified == false) {
    if(overrideFailString != "") {
      engine->log->addMessage(overrideFailString);
    } else {
      engine->log->addMessage("Was that supposed to do something?");
    }
  }
}

//SpellCastRetData ScrollOfEnfeebleEnemies::specificRead(
//  Engine* const engine) {
//  return SpellEnfeeble().cast(engine->player, engine);
//
////  engine->player->getSpotedEnemies();
////  const vector<Actor*>& actors = engine->player->spotedEnemies;
////
////  if(actors.empty() == false) {
////    vector<Pos> actorPositions;
////
////    for(unsigned int i = 0; i < actors.size(); i++) {
////      actorPositions.push_back(actors.at(i)->pos);
////    }
////
////    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
////      actorPositions, clrMagenta, BLAST_ANIMATION_DELAY_FACTOR);
////
////    StatusEffect* const effect = getStatusEffect(engine);
////
////    for(unsigned int i = 0; i < actors.size(); i++) {
////      actors.at(i)->getStatusHandler()->tryAddEffect(effect->copy());
////    }
////
////    delete effect;
////
////    setRealDefinitionNames(engine, false);
////
////  } else {
////    failedToLearnRealName(engine);
////  }
//}

////void ScrollOfVoidChain::specificRead(Engine* const engine) {
////  setRealDefinitionNames(engine, false);
////  if(engine->player->getStatusHandler()->allowAct()) {
////    engine->marker->place(markerTask_spellVoidChain);
////  } else {
////    engine->log->addMessage("My spell is disrupted.");
////  }
////}
//
////void ScrollOfVoidChain::castAt(const Pos& pos, Engine* const engine) {
////  const Pos playerPos = engine->player->pos;
////  const vector<Pos> projectilePath =
////    engine->mapTests->getLine(playerPos.x, playerPos.y, pos.x, pos.y,
////                              true, FOV_STANDARD_RADI_INT);
////}
//
////void ScrollOfIbnGhazisPowder::specificRead(Engine* const engine) {
////  setRealDefinitionNames(engine, false);
////  if(engine->player->getStatusHandler()->allowAct()) {
////    engine->query->direction();
////  } else {
////    engine->log->addMessage("My spell is disrupted.");
////  }
////}

ScrollNameHandler::ScrollNameHandler(Engine* engine) :
  eng(engine) {
  m_falseNames.resize(0);
  m_falseNames.push_back("Cruensseasrjit");
  m_falseNames.push_back("Rudsceleratus");
  m_falseNames.push_back("Rudminuox");
  m_falseNames.push_back("Cruo-stragaraNa");
  m_falseNames.push_back("PrayaNavita");
  m_falseNames.push_back("Pretiacruento");
  m_falseNames.push_back("Pestis cruento");
  m_falseNames.push_back("Cruento pestis");
  m_falseNames.push_back("Domus-bhaava");
  m_falseNames.push_back("Acerbus-shatruex");
  m_falseNames.push_back("Pretaanluxis");
  m_falseNames.push_back("PraaNsilenux");
  m_falseNames.push_back("Quodpipax");
  m_falseNames.push_back("Lokemundux");
  m_falseNames.push_back("Profanuxes");
  m_falseNames.push_back("Shaantitus");
  m_falseNames.push_back("Geropayati");
  m_falseNames.push_back("Vilomaxus");
  m_falseNames.push_back("Bhuudesco");
  m_falseNames.push_back("Durbentia");
  m_falseNames.push_back("Bhuuesco");
  m_falseNames.push_back("Maravita");
  m_falseNames.push_back("Infirmux");

  vector<string> cmb;
  cmb.resize(0);
  cmb.push_back("Cruo");
  cmb.push_back("Cruonit");
  cmb.push_back("Cruentu");
  cmb.push_back("Marana");
  cmb.push_back("Domus");
  cmb.push_back("Malax");
  cmb.push_back("Caecux");
  cmb.push_back("Eximha");
  cmb.push_back("Vorox");
  cmb.push_back("Bibox");
  cmb.push_back("Pallex");
  cmb.push_back("Profanx");
  cmb.push_back("Invisuu");
  cmb.push_back("Invisux");
  cmb.push_back("Odiosuu");
  cmb.push_back("Odiosux");
  cmb.push_back("Vigra");
  cmb.push_back("Crudux");
  cmb.push_back("Desco");
  cmb.push_back("Esco");
  cmb.push_back("Gero");
  cmb.push_back("Klaatu");
  cmb.push_back("Barada");
  cmb.push_back("Nikto");

  const unsigned int CMB_SIZ = cmb.size();
  for(unsigned int i = 0; i < CMB_SIZ; i++) {
    for(unsigned int ii = 0; ii < CMB_SIZ; ii++) {
      if(i != ii) {
        m_falseNames.push_back(cmb.at(i) + " " + cmb.at(ii));
      }
    }
  }
}

void ScrollNameHandler::setFalseScrollName(ItemDef* d) {
  const unsigned int NR_NAMES = m_falseNames.size();

  const unsigned int ELEMENT = (unsigned int)(eng->dice(1, NR_NAMES) - 1);

  const string TITLE = "\"" + m_falseNames.at(ELEMENT) + "\"";

  m_falseNames.erase(m_falseNames.begin() + ELEMENT);

  d->name.name = "Manuscript titled " + TITLE;
  d->name.name_plural = "Manuscripts titled " + TITLE;
  d->name.name_a = "a Manuscript titled " + TITLE;
}

void ScrollNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    if(eng->itemData->itemDefs[i]->isReadable == true) {
      lines.push_back(eng->itemData->itemDefs[i]->name.name);
      lines.push_back(eng->itemData->itemDefs[i]->name.name_plural);
      lines.push_back(eng->itemData->itemDefs[i]->name.name_a);
    }
  }
}

void ScrollNameHandler::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    if(eng->itemData->itemDefs[i]->isReadable == true) {
      eng->itemData->itemDefs[i]->name.name = lines.front();
      lines.erase(lines.begin());
      eng->itemData->itemDefs[i]->name.name_plural = lines.front();
      lines.erase(lines.begin());
      eng->itemData->itemDefs[i]->name.name_a = lines.front();
      lines.erase(lines.begin());
    }
  }
}
