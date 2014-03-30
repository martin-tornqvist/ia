#include "ItemScroll.h"

#include "Engine.h"

#include "ActorPlayer.h"
#include "Map.h"
#include "PlayerBonuses.h"
#include "Log.h"
#include "Inventory.h"
#include "PlayerSpellsHandler.h"
#include "Renderer.h"
#include "Utils.h"

const string Scroll::getRealTypeName() {
  switch(data_->spellCastFromScroll) {
    case spell_darkbolt:            return "Darkbolt";
    case spell_azathothsWrath:      return "Azathoths Wrath";
    case spell_enfeeble:            return "Enfeeblement";
    case spell_teleport:            return "Teleportation";
    case spell_bless:               return "Blessing";
    case spell_mayhem:              return "Mayhem";
    case spell_pestilence:          return "Pestilence";
    case spell_detectItems:         return "Detect Items";
    case spell_detectTraps:         return "Detect Traps";
    case spell_detectMonsters:      return "Detect Monsters";
    case spell_opening:             return "Opening";
    case spell_sacrificeLife:       return "Sacrifice Life Force";
    case spell_sacrificeSpirit:     return "Sacrifice Spirit";
    case spell_disease:             return "";
    case spell_summonRandom:        return "";
    case spell_healSelf:            return "";
    case spell_knockBack:           return "";
    case spell_cloudMinds:          return "";
    case spell_miGoHypnosis:        return "";
    case endOfSpellId: {} break;
  }
  assert(false && "No spell found for scroll");
  return "";
}

bool Scroll::activateDefault(Actor* const actor) {
  if(actor->getPropHandler().allowRead(true)) {return read();}
  return false;
}

Spell* Scroll::getSpell() {
  return eng.spellHandler->getSpellFromId(data_->spellCastFromScroll);
}

void Scroll::identify(const bool IS_SILENT_IDENTIFY) {
  if(data_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME        = "Manuscript of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Manuscripts of " + REAL_TYPE_NAME;
    const string REAL_NAME_A      = "a Manuscript of " + REAL_TYPE_NAME;

    data_->name.name = REAL_NAME;
    data_->name.name_plural = REAL_NAME_PLURAL;
    data_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      eng.log->addMsg("It was " + data_->name.name_a + ".");
      Renderer::drawMapAndInterface();
    }

    data_->isIdentified = true;
  }
}

void Scroll::tryLearn() {
  if(eng.playerBonHandler->getBg() == bgOccultist) {
    Spell* const spell = getSpell();
    if(
      spell->isAvailForPlayer() &&
      eng.playerSpellsHandler->isSpellLearned(spell->getId()) == false) {
      eng.log->addMsg("I learn to cast this incantation by heart!");
      eng.playerSpellsHandler->learnSpellIfNotKnown(spell);
    } else {
      delete spell;
    }
  }
}

bool Scroll::read() {
  Renderer::drawMapAndInterface();

  if(eng.player->getPropHandler().allowSee() == false) {
    eng.log->addMsg("I cannot read while blind.");
    return false;
  }

  Spell* const spell = getSpell();

  if(data_->isIdentified) {
    eng.log->addMsg(
      "I read a scroll of " + getRealTypeName() + "...");
    spell->cast(eng.player, false, eng);
    tryLearn();
  } else {
    eng.log->addMsg("I recite forbidden incantations...");
    data_->isTried = true;
    if(spell->cast(eng.player, false, eng).isCastIdenifying) {
      identify(false);
    }
  }
  delete spell;

  return true;
}

ScrollNameHandler::ScrollNameHandler(Engine& engine) :
  eng(engine) {
  falseNames_.resize(0);
  falseNames_.push_back("Cruensseasrjit");
  falseNames_.push_back("Rudsceleratus");
  falseNames_.push_back("Rudminuox");
  falseNames_.push_back("Cruo-stragaraNa");
  falseNames_.push_back("PrayaNavita");
  falseNames_.push_back("Pretiacruento");
  falseNames_.push_back("Pestis cruento");
  falseNames_.push_back("Cruento pestis");
  falseNames_.push_back("Domus-bhaava");
  falseNames_.push_back("Acerbus-shatruex");
  falseNames_.push_back("Pretaanluxis");
  falseNames_.push_back("PraaNsilenux");
  falseNames_.push_back("Quodpipax");
  falseNames_.push_back("Lokemundux");
  falseNames_.push_back("Profanuxes");
  falseNames_.push_back("Shaantitus");
  falseNames_.push_back("Geropayati");
  falseNames_.push_back("Vilomaxus");
  falseNames_.push_back("Bhuudesco");
  falseNames_.push_back("Durbentia");
  falseNames_.push_back("Bhuuesco");
  falseNames_.push_back("Maravita");
  falseNames_.push_back("Infirmux");

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

  const int CMB_SIZ = cmb.size();
  for(int i = 0; i < CMB_SIZ; i++) {
    for(int ii = 0; ii < CMB_SIZ; ii++) {
      if(i != ii) {
        falseNames_.push_back(cmb.at(i) + " " + cmb.at(ii));
      }
    }
  }
}

void ScrollNameHandler::setFalseScrollName(ItemData& d) {
  const int NR_ELEMENTS = falseNames_.size();
  const int ELEMENT     = Rnd::range(0, NR_ELEMENTS - 1);

  const string& TITLE = "\"" + falseNames_.at(ELEMENT) + "\"";

  d.name.name         = "Manuscript titled " + TITLE;
  d.name.name_plural  = "Manuscripts titled " + TITLE;
  d.name.name_a       = "a Manuscript titled " + TITLE;

  falseNames_.erase(falseNames_.begin() + ELEMENT);
}

void ScrollNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    if(eng.itemDataHandler->dataList[i]->isScroll) {
      lines.push_back(eng.itemDataHandler->dataList[i]->name.name);
      lines.push_back(eng.itemDataHandler->dataList[i]->name.name_plural);
      lines.push_back(eng.itemDataHandler->dataList[i]->name.name_a);
    }
  }
}

void ScrollNameHandler::setParamsFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    if(eng.itemDataHandler->dataList[i]->isScroll) {
      eng.itemDataHandler->dataList[i]->name.name = lines.front();
      lines.erase(lines.begin());
      eng.itemDataHandler->dataList[i]->name.name_plural = lines.front();
      lines.erase(lines.begin());
      eng.itemDataHandler->dataList[i]->name.name_a = lines.front();
      lines.erase(lines.begin());
    }
  }
}
