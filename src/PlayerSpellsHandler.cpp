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
      const MenuAction_t action = eng.menuInputHandler->getAction(browser);
      switch(action) {
        case menuAction_browsed: {
          draw(browser);
        }
        break;
        case menuAction_canceled: {
          eng.log->clearLog();
          eng.renderer->drawMapAndInterface();
          return;
        }
        break;
        case menuAction_selected: {
          eng.log->clearLog();
          eng.renderer->drawMapAndInterface();
          knownSpells.at(browser.getPos().y)->cast(eng.player, true, eng);
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

    string fill;
    fill.resize(0);
    const unsigned int FILL_SIZE = 28 - str.size();
    for(unsigned int ii = 0; ii < FILL_SIZE; ii++) {fill.push_back('.');}
    SDL_Color fillClr = clrGray;
    fillClr.r /= 3; fillClr.g /= 3; fillClr.b /= 3;
    eng.renderer->drawText(
      fill, panel_screen, Pos(str.size(), y), fillClr);
    const int x = 28;

    const Range spiCost = spell->getSpiCost(false, eng.player, eng);

    str = "SPI:";
    str += spiCost.upper == 1 ? "1" :
           (toString(spiCost.lower) +  "-" + toString(spiCost.upper));
    eng.renderer->drawText(str, panel_screen, Pos(x, y), clrWhite);
    y++;
  }

  eng.renderer->updateScreen();
}

bool PlayerSpellsHandler::isSpellLearned(const Spell_t id) {
  for(unsigned int i = 0; i < knownSpells.size(); i++) {
    if(knownSpells.at(i)->getId() == id) {
      return true;
    }
  }
  return false;
}

void PlayerSpellsHandler::learnSpellIfNotKnown(const Spell_t id) {
  learnSpellIfNotKnown(eng.spellHandler->getSpellFromId(id));
}

void PlayerSpellsHandler::learnSpellIfNotKnown(Spell* const spell) {
  bool isAlreadyLearned = false;
  for(Spell* spellCmpr : knownSpells) {
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

