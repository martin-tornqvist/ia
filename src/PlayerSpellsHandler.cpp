#include "PlayerSpellsHandler.h"

#include "Engine.h"
#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Render.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "PlayerBonuses.h"

PlayerSpellsHandler::~PlayerSpellsHandler() {
  for(unsigned int i = 0; i < learnedSpells.size(); i++) {
    delete learnedSpells.at(i);
  }
}

void PlayerSpellsHandler::run() {
  if(learnedSpells.empty()) {
    eng->log->addMsg("I do not know any spells to invoke.");
  } else {
    MenuBrowser browser(learnedSpells.size(), 0);

    eng->renderer->drawMapAndInterface();

    draw(browser);

    while(true) {
      const MenuAction_t action = eng->menuInputHandler->getAction(browser);
      switch(action) {
        case menuAction_browsed: {
          draw(browser);
        }
        break;
        case menuAction_canceled: {
          eng->log->clearLog();
          eng->renderer->drawMapAndInterface();
          return;
        }
        break;
        case menuAction_selected: {
          eng->log->clearLog();
          eng->renderer->drawMapAndInterface();
          learnedSpells.at(browser.getPos().y)->cast(eng->player, true, eng);
          return;
        }
        break;
        default:
        {} break;
      }
    }
  }
}

void PlayerSpellsHandler::draw(MenuBrowser& browser) {
  const int NR_SPELLS = learnedSpells.size();
  string endLetter = "a";
  endLetter[0] += char(NR_SPELLS - 1);

  const string label =
    "Choose a memorized power to evoke [a-" + endLetter + "]" +
    " | Space/esc to exit";
  eng->renderer->drawText(label, panel_screen, Pos(1, 1), clrWhiteHigh);

  int currentListPos = 0;

  for(int i = 0; i < NR_SPELLS; i++) {
    const char CURRENT_KEY = 'a' + currentListPos;
    const SDL_Color clr =
      browser.isPosAtKey(CURRENT_KEY) ? clrWhite : clrRedLgt;
    Spell* const spell = learnedSpells.at(i);
    const string name = spell->getName();
    string str = "a";
    str[0] = CURRENT_KEY;
    str += ") " + name;

    eng->renderer->drawText(str, panel_map, Pos(1, currentListPos), clr);

    string fill;
    fill.resize(0);
    const unsigned int FILL_SIZE = 28 - str.size();
    for(unsigned int ii = 0; ii < FILL_SIZE; ii++) {
      fill.push_back('.');
    }
    SDL_Color fillClr = clrGray;
    fillClr.r /= 3; fillClr.g /= 3; fillClr.b /= 3;
    eng->renderer->drawText(
      fill, panel_map, Pos(1 + str.size(), currentListPos), fillClr);
    const int x = 28;

    const Range spiCost = spell->getSpiCost(false, eng->player, eng);

    str = "SPI:";
    str += spiCost.upper == 1 ? "1" :
           (intToString(spiCost.lower) +  "-" + intToString(spiCost.upper));
    eng->renderer->drawText(str, panel_map, Pos(x, currentListPos), clrWhite);
    currentListPos++;
  }

  eng->renderer->updateScreen();
}

bool PlayerSpellsHandler::isSpellLearned(const Spells_t id) {
  for(unsigned int i = 0; i < learnedSpells.size(); i++) {
    if(learnedSpells.at(i)->getId() == id) {
      return true;
    }
  }
  return false;
}

void PlayerSpellsHandler::learnSpellIfNotKnown(const Spells_t id) {
  learnSpellIfNotKnown(eng->spellHandler->getSpellFromId(id));
}

void PlayerSpellsHandler::learnSpellIfNotKnown(Spell* const spell) {
  bool isAlreadyLearned = false;
  for(unsigned int i = 0; i < learnedSpells.size(); i++) {
    if(learnedSpells.at(i)->getId() == spell->getId()) {
      isAlreadyLearned = true;
      break;
    }
  }
  if(isAlreadyLearned) {
    delete spell;
  } else {
    learnedSpells.push_back(spell);
  }
}

