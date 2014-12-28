#include "PlayerSpellsHandling.h"

#include <vector>
#include <algorithm>

#include "Init.h"
#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "MenuInputHandling.h"
#include "Render.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "PlayerBon.h"
#include "Query.h"
#include "Utils.h"
#include "Map.h"

using namespace std;

namespace PlayerSpellsHandling
{

namespace
{

struct SpellOpt
{
  SpellOpt() :
    spell(nullptr),
    srcItem(nullptr) {}

  SpellOpt(Spell* spell_, Item* srcItem_) :
    spell(spell_),
    srcItem(srcItem_) {}

  Spell*  spell;
  Item*   srcItem;
};

vector<Spell*>  knownSpells_;
SpellOpt        prevCast_;

void draw(MenuBrowser& browser, const vector<SpellOpt>& spellOpts)
{
  const int NR_SPELLS = spellOpts.size();

  Render::clearScreen();

  const string label = "Invoke which power? [enter] to cast " + cancelInfoStr;
  Render::drawText(label, Panel::screen, Pos(0, 0), clrWhiteHigh);

  for (int i = 0; i < NR_SPELLS; ++i)
  {
    const int     CURRENT_ELEMENT = i;
    Scroll        scroll(nullptr);
    Clr           scrollClr   = scroll.getInterfaceClr();
    const bool    IS_SELECTED = browser.isAtIdx(CURRENT_ELEMENT);
    const Clr     clr         = IS_SELECTED ? clrWhite : scrollClr;
    SpellOpt      spellOpt    = spellOpts[i];
    Spell* const  spell       = spellOpt.spell;
    string        name        = spell->getName();
    const int     NAME_X      = 1;
    const int     SPI_X       = 26;
    const int     SHOCK_X     = SPI_X + 10;
    const int     Y           = 2 + i;

    Render::drawText(name, Panel::screen, Pos(NAME_X, Y), clr);

    string fillStr = "";
    const size_t FILL_SIZE = SPI_X - NAME_X - name.size();
    for (size_t ii = 0; ii < FILL_SIZE; ii++) {fillStr.push_back('.');}
    Clr fillClr = clrGray;
    fillClr.r /= 3; fillClr.g /= 3; fillClr.b /= 3;
    Render::drawText(fillStr, Panel::screen, Pos(NAME_X + name.size(), Y), fillClr);

    int     x       = SPI_X;
    string  infoStr = "SPI: ";

    const Range spiCost = spell->getSpiCost(false, Map::player);
    const string lowerStr = toStr(spiCost.lower);
    const string upperStr = toStr(spiCost.upper);
    infoStr += spiCost.upper == 1 ? "1" : (lowerStr +  "-" + upperStr);

    Render::drawText(infoStr, Panel::screen, Pos(x, Y), clrWhite);

    x = SHOCK_X;
    const IntrSpellShock shockType = spell->getShockTypeIntrCast();
    switch (shockType)
    {
      case IntrSpellShock::mild:        infoStr = "Mild";       break;
      case IntrSpellShock::disturbing:  infoStr = "Disturbing"; break;
      case IntrSpellShock::severe:      infoStr = "Severe";     break;
    }
    Render::drawText(infoStr, Panel::screen, Pos(x, Y), clrWhite);

    if (IS_SELECTED)
    {
      const auto descr = spell->getDescr();
      vector<StrAndClr> lines;
      if (!descr.empty())
      {
        for (const auto& line : descr)
        {
          lines.push_back({line, clrWhiteHigh});
        }
      }
      //If spell source is an item, add info about this
      if (spellOpt.srcItem)
      {
        const string itemName =
          spellOpt.srcItem->getName(ItemRefType::plain, ItemRefInf::none);
        lines.push_back({"Spell granted by " + itemName + ".", clrGreen});
      }
      if (!lines.empty())
      {
        Render::drawDescrBox(lines);
      }
    }
  }

  Render::drawPopupBox(Rect(Pos(0, 1), Pos(DESCR_X0 - 1, SCREEN_H - 1)));

  Render::updateScreen();
}

void getSpellsAvail(vector<SpellOpt>& out)
{
  out.clear();

  for (Spell* const spell : knownSpells_)
  {
    out.push_back(SpellOpt(spell, nullptr));
  }

  Inventory& inv = Map::player->getInv();

  for (auto slot : inv.slots_)
  {
    Item* item = slot.item;
    if (item)
    {
      for (Spell* spell : item->carrierSpells_)
      {
        out.push_back(SpellOpt(spell, item));
      }
    }
  }

  for (Item* item : inv.general_)
  {
    for (Spell* spell : item->carrierSpells_)
    {
      out.push_back(SpellOpt(spell, item));
    }
  }
}

void tryCast(const SpellOpt& spellOpt)
{
  assert(spellOpt.spell);

  //TODO: It should be allowed to cast non-read spells while blind etc.
  //There should be something like "allowRead()" and allowCastSpells()".
  if (Map::player->getPropHandler().allowRead(true))
  {
    Log::clearLog();
    Render::drawMapAndInterface();

    Spell* const spell = spellOpt.spell;

    const Range spiCost = spell->getSpiCost(false, Map::player);
    if (spiCost.upper >= Map::player->getSpi())
    {
      Log::addMsg("Cast spell and risk depleting your spirit [y/n]?", clrWhiteHigh);
      Render::drawMapAndInterface();
      if (Query::yesOrNo() == YesNoAnswer::no)
      {
        Log::clearLog();
        Render::drawMapAndInterface();
        return;
      }
      Log::clearLog();
    }

    const bool IS_BLOOD_SORC = PlayerBon::traitsPicked[int(Trait::bloodSorcerer)];
    const bool IS_WARLOCK    = PlayerBon::traitsPicked[int(Trait::warlock)];

    const int BLOOD_SORC_HP_DRAINED = 2;

    if (IS_BLOOD_SORC)
    {
      if (Map::player->getHp() <= BLOOD_SORC_HP_DRAINED)
      {
        Log::addMsg("I do not have enough life force to cast this spell.");
        Render::drawMapAndInterface();
        return;
      }
    }

    Log::addMsg("I cast " + spell->getName() + "!");

    if (IS_BLOOD_SORC)
    {
      Map::player->hit(BLOOD_SORC_HP_DRAINED, DmgType::pure);
    }
    if (Map::player->isAlive())
    {
      spell->cast(Map::player, true);
      prevCast_ = spellOpt;
      if (IS_WARLOCK && Rnd::oneIn(2))
      {
        auto* const prop = new PropWarlockCharged(PropTurns::std);
        Map::player->getPropHandler().tryApplyProp(prop);
      }
    }
  }
}

} //PlayerSpellsHandling

void init()
{
  cleanup();
}

void cleanup()
{
  for (Spell* spell : knownSpells_) {delete spell;}
  knownSpells_.clear();
  prevCast_ = SpellOpt();
}

void storeToSaveLines(vector<string>& lines)
{
  lines.push_back(toStr(knownSpells_.size()));
  for (Spell* s : knownSpells_) {lines.push_back(toStr(int(s->getId())));}
}

void setupFromSaveLines(vector<string>& lines)
{
  const int NR_SPELLS = toInt(lines.front());
  lines.erase(begin(lines));

  for (int i = 0; i < NR_SPELLS; ++i)
  {
    const int ID = toInt(lines.front());
    lines.erase(begin(lines));
    knownSpells_.push_back(SpellHandling::mkSpellFromId(SpellId(ID)));
  }
}

void playerSelectSpellToCast()
{
  vector<SpellOpt> spellOpts;
  getSpellsAvail(spellOpts);

  if (spellOpts.empty())
  {
    Log::addMsg("I do not know any spells to invoke.");
  }
  else
  {
    auto spellOptSort = [](const SpellOpt & opt1, const SpellOpt & opt2)
    {
      return opt1.spell->getName() < opt2.spell->getName();
    };

    sort(spellOpts.begin(), spellOpts.end(), spellOptSort);

    MenuBrowser browser(spellOpts.size(), 0);

    Render::drawMapAndInterface();

    draw(browser, spellOpts);

    while (true)
    {
      const MenuAction action = MenuInputHandling::getAction(browser);
      switch (action)
      {
        case MenuAction::browsed:
          draw(browser, spellOpts);
          break;

        case MenuAction::esc:
        case MenuAction::space:
          Log::clearLog();
          Render::drawMapAndInterface();
          return;

        case MenuAction::selected:
          tryCast(spellOpts[browser.getPos().y]);
          return;

        default: {}
          break;
      }
    }
  }
}

void tryCastPrevSpell()
{
  TRACE_FUNC_BEGIN;

  bool isPrevSpellOk = false;

  if (prevCast_.spell)
  {
    //Checking if previous spell is still available (it could for example have been
    //granted by an item that was dropped)
    vector<SpellOpt> spellOpts;
    getSpellsAvail(spellOpts);

    auto spellOptCmp = [&](const SpellOpt & opt) {return opt.spell == prevCast_.spell;};

    isPrevSpellOk = find_if(begin(spellOpts), end(spellOpts), spellOptCmp) !=
                    end(spellOpts);
  }

  if (isPrevSpellOk)
  {
    TRACE << "Previous spell is available, casting" << endl;
    tryCast(prevCast_);
  }
  else
  {
    TRACE << "No previous spell set, player picks spell instead" << endl;
    playerSelectSpellToCast();
  }

  TRACE_FUNC_END;
}

bool isSpellLearned(const SpellId id)
{
  for (auto* s : knownSpells_) {if (s->getId() == id) {return true;}}
  return false;
}

void learnSpellIfNotKnown(const SpellId id)
{
  learnSpellIfNotKnown(SpellHandling::mkSpellFromId(id));
}

void learnSpellIfNotKnown(Spell* const spell)
{
  bool isAlreadyLearned = false;
  for (Spell* spellCmpr : knownSpells_)
  {
    if (spellCmpr->getId() == spell->getId())
    {
      isAlreadyLearned = true;
      break;
    }
  }
  if (isAlreadyLearned)
  {
    delete spell;
  }
  else
  {
    knownSpells_.push_back(spell);
  }
}

} //PlayerSpellsHandling
