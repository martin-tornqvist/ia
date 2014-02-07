#include "PlayerSpellsHandler.h"

#include "Engine.h"
#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Renderer.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "PlayerBonuses.h"
#include "Query.h"

PlayerSpellsHandler::~PlayerSpellsHandler() {
  for(Spell * spell : knownSpells) {delete spell;}
}

void PlayerSpellsHandler::run() {
  if(knownSpells.empty()) {
    eng.log->addMsg("I do not know any spells to invoke.");
  } else {

    sort(knownSpells.begin(), knownSpells.end(), [](Spell * s1, Spell * s2) {
      return s1->getName() < s2->getName();
    });

    MenuBrowser browser(knownSpells.size(), 0);

    eng.renderer->drawMapAndInterface();

    draw(browser);

    while(true) {
      const MenuAction action = eng.menuInputHandler->getAction(browser);
      switch(action) {
        case menuAction_browsed: {
          draw(browser);
        } break;

        case menuAction_esc:
        case menuAction_space: {
          eng.log->clearLog();
          eng.renderer->drawMapAndInterface();
          return;
        } break;

        case menuAction_selected: {
          eng.log->clearLog();
          eng.renderer->drawMapAndInterface();

          Spell* spell = knownSpells.at(browser.getPos().y);

          const Range spiCost = spell->getSpiCost(false, eng.player, eng);
          if(spiCost.upper >= eng.player->getSpi()) {
            eng.log->addMsg("Cast spell and risk depleting your spirit (y/n)?",
                            clrWhiteHigh);
            eng.renderer->drawMapAndInterface();
            if(eng.query->yesOrNo() == false) {
              eng.log->clearLog();
              eng.renderer->drawMapAndInterface();
              return;
            }
          }

          spell->cast(eng.player, true, eng);
          return;
        } break;

        default: {} break;
      }
    }
  }
}

void PlayerSpellsHandler::draw(MenuBrowser& browser) {
  const int NR_SPELLS = knownSpells.size();
  string endLetter = "a";
  endLetter[0] += char(NR_SPELLS - 1);

  const string label =
    "Choose a power to evoke [a-" + endLetter + "]" +
    " | Space/esc to exit";
  eng.renderer->drawText(label, panel_screen, Pos(0, 0), clrWhiteHigh);

  int y = 1;

  for(int i = 0; i < NR_SPELLS; i++) {
    const char CURRENT_KEY = 'a' + i;
    const SDL_Color clr =
      browser.isPosAtKey(CURRENT_KEY) ? clrWhite : clrRedLgt;
    Spell* const spell = knownSpells.at(i);
    const string name = spell->getName();
    string str = "a";
    str[0] = CURRENT_KEY;
    str += ") " + name;

    eng.renderer->drawText(str, panel_screen, Pos(0, y), clr);

    string fillStr = "";
    const unsigned int FILL_SIZE = 28 - str.size();
    for(unsigned int ii = 0; ii < FILL_SIZE; ii++) {fillStr.push_back('.');}
    SDL_Color fillClr = clrGray;
    fillClr.r /= 3; fillClr.g /= 3; fillClr.b /= 3;
    eng.renderer->drawText(fillStr, panel_screen, Pos(str.size(), y), fillClr);

    int x = 28;
    str = "SPI:";
    const Range spiCost = spell->getSpiCost(false, eng.player, eng);
    const string lowerStr = toString(spiCost.lower);
    const string upperStr = toString(spiCost.upper);
    str += spiCost.upper == 1 ? "1" : (lowerStr +  "-" + upperStr);
    eng.renderer->drawText(str, panel_screen, Pos(x, y), clrWhite);

    x += 10;
    str = "SHOCK: ";
    const IntrSpellShock shockType = spell->getShockTypeIntrCast();
    switch(shockType) {
      case intrSpellShockMild:        str += "Mild";       break;
      case intrSpellShockDisturbing:  str += "Disturbing"; break;
      case intrSpellShockSevere:      str += "Severe";     break;
    }
    eng.renderer->drawText(str, panel_screen, Pos(x, y), clrWhite);

    y++;
  }

  eng.renderer->updateScreen();
}

void PlayerSpellsHandler::addSaveLines(vector<string>& lines) const {
  lines.push_back(toString(knownSpells.size()));
  for(Spell * s : knownSpells) {lines.push_back(toString(s->getId()));}
}

void PlayerSpellsHandler::setParamsFromSaveLines(vector<string>& lines) {
  const int NR_SPELLS = toInt(lines.front());
  lines.erase(lines.begin());

  for(int i = 0; i < NR_SPELLS; i++) {
    const int ID = toInt(lines.front());
    lines.erase(lines.begin());
    knownSpells.push_back(eng.spellHandler->getSpellFromId(SpellId(ID)));
  }
}

bool PlayerSpellsHandler::isSpellLearned(const SpellId id) {
  for(Spell * s : knownSpells) {if(s->getId() == id) {return true;}}
  return false;
}

void PlayerSpellsHandler::learnSpellIfNotKnown(const SpellId id) {
  learnSpellIfNotKnown(eng.spellHandler->getSpellFromId(id));
}

void PlayerSpellsHandler::learnSpellIfNotKnown(Spell* const spell) {
  bool isAlreadyLearned = false;
  for(Spell * spellCmpr : knownSpells) {
    if(spellCmpr->getId() == spell->getId()) {
      isAlreadyLearned = true;
      break;
    }
  }
  if(isAlreadyLearned) {
    delete spell;
  } else {
    knownSpells.push_back(spell);
  }
}
