#include "ItemScroll.h"

#include <string>
#include <assert.h>

#include "ActorPlayer.h"
#include "Map.h"
#include "PlayerBon.h"
#include "Log.h"
#include "Inventory.h"
#include "PlayerSpellsHandling.h"
#include "Renderer.h"
#include "Utils.h"

using namespace std;

const string Scroll::getRealTypeName() {
  switch(data_->spellCastFromScroll) {
    case SpellId::darkbolt:           return "Darkbolt";
    case SpellId::azathothsWrath:     return "Azathoths Wrath";
    case SpellId::slowEnemies:        return "Slow Enemies";
    case SpellId::terrifyEnemies:     return "Terrify Enemies";
    case SpellId::paralyzeEnemies:    return "Paralyze Enemies";
    case SpellId::teleport:           return "Teleportation";
    case SpellId::bless:              return "Blessing";
    case SpellId::mayhem:             return "Mayhem";
    case SpellId::pestilence:         return "Pestilence";
    case SpellId::detItems:           return "Detect Items";
    case SpellId::detTraps:           return "Detect Traps";
    case SpellId::detMon:             return "Detect Monsters";
    case SpellId::opening:            return "Opening";
    case SpellId::sacrLife:           return "Sacrifice Life Force";
    case SpellId::sacrSpi:            return "Sacrifice Spirit";
    case SpellId::elemRes:            return "Elemental Resistance";
    case SpellId::disease:            return "";
    case SpellId::summonRandom:       return "";
    case SpellId::healSelf:           return "";
    case SpellId::knockBack:          return "";
    case SpellId::cloudMinds:         return "";
    case SpellId::miGoHypnosis:       return "";
    case SpellId::immolation:         return "";
    case SpellId::endOfSpellId: {} break;
  }
  assert(false && "No spell found for scroll");
  return "";
}

ConsumeItem Scroll::activateDefault(Actor* const actor) {
  if(actor->getPropHandler().allowRead(true)) {return read();}
  return ConsumeItem::no;
}

Spell* Scroll::getSpell() {
  return SpellHandling::getSpellFromId(data_->spellCastFromScroll);
}

void Scroll::identify(const bool IS_SILENT_IDENTIFY) {
  if(!data_->isIdentified) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME        = "Manuscript of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Manuscripts of " + REAL_TYPE_NAME;
    const string REAL_NAME_A      = "a Manuscript of " + REAL_TYPE_NAME;

    data_->baseName.name        = REAL_NAME;
    data_->baseName.name_plural = REAL_NAME_PLURAL;
    data_->baseName.nameA      = REAL_NAME_A;

    if(!IS_SILENT_IDENTIFY) {
      Log::addMsg("It was " + data_->baseName.nameA + ".");
      Renderer::drawMapAndInterface();
    }

    data_->isIdentified = true;
  }
}

void Scroll::tryLearn() {
  if(PlayerBon::getBg() == Bg::occultist) {
    Spell* const spell = getSpell();
    if(
      spell->isAvailForPlayer() &&
      !PlayerSpellsHandling::isSpellLearned(spell->getId())) {
      Log::addMsg("I learn to cast this incantation by heart!");
      PlayerSpellsHandling::learnSpellIfNotKnown(spell);
    } else {
      delete spell;
    }
  }
}

ConsumeItem Scroll::read() {
  Renderer::drawMapAndInterface();

  if(!Map::player->getPropHandler().allowSee()) {
    Log::addMsg("I cannot read while blind.");
    return ConsumeItem::no;
  }

  Spell* const spell = getSpell();

  if(data_->isIdentified) {
    Log::addMsg(
      "I read a scroll of " + getRealTypeName() + "...");
    spell->cast(Map::player, false);
    tryLearn();
  } else {
    Log::addMsg("I recite forbidden incantations...");
    data_->isTried = true;
    if(spell->cast(Map::player, false).isCastIdenifying) {
      identify(false);
    }
  }
  delete spell;

  return ConsumeItem::yes;
}

namespace ScrollNameHandling {

namespace {

vector<string> falseNames_;

} //namespace

void init() {
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

  const size_t NR_CMB_PARTS = cmb.size();
  for(size_t i = 0; i < NR_CMB_PARTS; ++i) {
    for(size_t ii = 0; ii < NR_CMB_PARTS; ii++) {
      if(i != ii) {
        falseNames_.push_back(cmb.at(i) + " " + cmb.at(ii));
      }
    }
  }
}

void setFalseScrollName(ItemDataT& d) {
  const int NR_ELEMENTS = falseNames_.size();
  const int ELEMENT     = Rnd::range(0, NR_ELEMENTS - 1);

  const string& TITLE = "\"" + falseNames_.at(ELEMENT) + "\"";

  d.baseName.name         = "Manuscript titled " + TITLE;
  d.baseName.name_plural  = "Manuscripts titled " + TITLE;
  d.baseName.nameA       = "a Manuscript titled " + TITLE;

  falseNames_.erase(falseNames_.begin() + ELEMENT);
}

void storeToSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::endOfItemIds); ++i) {
    if(ItemData::data[i]->isScroll) {
      lines.push_back(ItemData::data[i]->baseName.name);
      lines.push_back(ItemData::data[i]->baseName.name_plural);
      lines.push_back(ItemData::data[i]->baseName.nameA);
    }
  }
}

void setupFromSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::endOfItemIds); ++i) {
    if(ItemData::data[i]->isScroll) {
      ItemData::data[i]->baseName.name        = lines.front();
      lines.erase(begin(lines));
      ItemData::data[i]->baseName.name_plural = lines.front();
      lines.erase(begin(lines));
      ItemData::data[i]->baseName.nameA      = lines.front();
      lines.erase(begin(lines));
    }
  }
}

} //ScrollNameHandling
