#include "ItemScroll.h"

#include <string>
#include <assert.h>

#include "Init.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "PlayerBon.h"
#include "Log.h"
#include "Inventory.h"
#include "PlayerSpellsHandling.h"
#include "Render.h"
#include "Utils.h"
#include "ItemFactory.h"

using namespace std;

const string Scroll::getRealName() const
{
  Spell* spell      = SpellHandling::mkSpellFromId(data_->spellCastFromScroll);
  const string name = spell->getName();
  delete spell;
  return name;
}

vector<string> Scroll::getDescr() const
{
  if (data_->isIdentified)
  {
    const auto* const spell = mkSpell();
    const auto descr = spell->getDescr();
    delete spell;
    return descr;
  }
  else
  {
    return data_->baseDescr;
  }
}

ConsumeItem Scroll::activateDefault(Actor* const actor)
{
  if (actor->getPropHandler().allowRead(true)) {return read();}
  return ConsumeItem::no;
}

Spell* Scroll::mkSpell() const
{
  return SpellHandling::mkSpellFromId(data_->spellCastFromScroll);
}

void Scroll::identify(const bool IS_SILENT_IDENTIFY)
{
  if (!data_->isIdentified)
  {

    data_->isIdentified = true;

    if (!IS_SILENT_IDENTIFY)
    {
      const string name = getName(ItemRefType::a, ItemRefInf::none);
      Log::addMsg("It was " + name + ".");
      Render::drawMapAndInterface();
    }
  }
}

void Scroll::tryLearn()
{
  if (PlayerBon::getBg() == Bg::occultist)
  {
    Spell* const spell = mkSpell();
    if (spell->isAvailForPlayer() &&
        !PlayerSpellsHandling::isSpellLearned(spell->getId()))
    {
      Log::addMsg("I learn to cast this incantation by heart!");
      PlayerSpellsHandling::learnSpellIfNotKnown(spell);
    }
    else
    {
      delete spell;
    }
  }
}

ConsumeItem Scroll::read()
{
  Render::drawMapAndInterface();

  if (!Map::player->getPropHandler().allowSee())
  {
    Log::addMsg("I cannot read while blind.");
    return ConsumeItem::no;
  }

  auto* const spell = mkSpell();

  const string crumbleStr = "It crumbles to dust.";

  if (data_->isIdentified)
  {
    const string name = getName(ItemRefType::a, ItemRefInf::none);
    Log::addMsg("I read " + name + "...");
    spell->cast(Map::player, false);
    Log::addMsg(crumbleStr);
    tryLearn();
  }
  else
  {
    Log::addMsg("I recite the forbidden incantations on the manuscript...");
    data_->isTried = true;
    const auto isNoticed = spell->cast(Map::player, false);
    Log::addMsg(crumbleStr);
    if (isNoticed == SpellEffectNoticed::yes) {identify(false);}
  }

  delete spell;

  return ConsumeItem::yes;
}

string Scroll::getNameInf() const
{
  return (data_->isTried && !data_->isIdentified) ? "{Tried}" : "";
}

namespace ScrollNameHandling
{

namespace
{

vector<string> falseNames_;

} //namespace

void init()
{
  TRACE_FUNC_BEGIN;

  //Init possible fake names
  falseNames_.clear();
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
  cmb.clear();
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
  for (size_t i = 0; i < NR_CMB_PARTS; ++i)
  {
    for (size_t ii = 0; ii < NR_CMB_PARTS; ii++)
    {
      if (i != ii)
      {
        falseNames_.push_back(cmb[i] + " " + cmb[ii]);
      }
    }
  }

  TRACE << "Init scroll names" << endl;
  for (auto* const d : ItemData::data)
  {
    if (d->isScroll)
    {
      //False name
      const int NR_ELEMENTS = falseNames_.size();
      const int ELEMENT     = Rnd::range(0, NR_ELEMENTS - 1);

      const string& TITLE = falseNames_[ELEMENT];

      d->baseNameUnid.names[int(ItemRefType::plain)]   = "Manuscript titled "    + TITLE;
      d->baseNameUnid.names[int(ItemRefType::plural)]  = "Manuscripts titled "   + TITLE;
      d->baseNameUnid.names[int(ItemRefType::a)]       = "a Manuscript titled "  + TITLE;

      falseNames_.erase(falseNames_.begin() + ELEMENT);

      //True name
      const Scroll* const scroll =
        static_cast<const Scroll*>(ItemFactory::mk(d->id, 1));

      const string REAL_TYPE_NAME = scroll->getRealName();

      delete scroll;

      const string REAL_NAME        = "Manuscript of "    + REAL_TYPE_NAME;
      const string REAL_NAME_PLURAL = "Manuscripts of "   + REAL_TYPE_NAME;
      const string REAL_NAME_A      = "a Manuscript of "  + REAL_TYPE_NAME;

      d->baseName.names[int(ItemRefType::plain)]  = REAL_NAME;
      d->baseName.names[int(ItemRefType::plural)] = REAL_NAME_PLURAL;
      d->baseName.names[int(ItemRefType::a)]      = REAL_NAME_A;
    }
  }

  TRACE_FUNC_END;
}

void storeToSaveLines(vector<string>& lines)
{
  for (int i = 0; i < int(ItemId::END); ++i)
  {
    if (ItemData::data[i]->isScroll)
    {
      lines.push_back(ItemData::data[i]->baseNameUnid.names[int(ItemRefType::plain)]);
      lines.push_back(ItemData::data[i]->baseNameUnid.names[int(ItemRefType::plural)]);
      lines.push_back(ItemData::data[i]->baseNameUnid.names[int(ItemRefType::a)]);
    }
  }
}

void setupFromSaveLines(vector<string>& lines)
{
  for (int i = 0; i < int(ItemId::END); ++i)
  {
    if (ItemData::data[i]->isScroll)
    {
      ItemData::data[i]->baseNameUnid.names[int(ItemRefType::plain)]  = lines.front();
      lines.erase(begin(lines));
      ItemData::data[i]->baseNameUnid.names[int(ItemRefType::plural)] = lines.front();
      lines.erase(begin(lines));
      ItemData::data[i]->baseNameUnid.names[int(ItemRefType::a)]      = lines.front();
      lines.erase(begin(lines));
    }
  }
}

} //ScrollNameHandling
