#include "PlayerPowersHandler.h"

#include "Engine.h"
#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Render.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "PlayerBonuses.h"

PlayerPowersHandler::PlayerPowersHandler(Engine* engine) :
  eng(engine) {

  for(unsigned int i = 1; i < endOfItemIds; i++) {
    const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isScroll) {
      scrollsToReadFromPlayerMemory.push_back(
        dynamic_cast<Scroll*>(eng->itemFactory->spawnItem(static_cast<ItemId_t>(i))));
    }
  }
}

PlayerPowersHandler::~PlayerPowersHandler() {
  for(unsigned int i = 0; i < scrollsToReadFromPlayerMemory.size(); i++) {
    delete scrollsToReadFromPlayerMemory.at(i);
  }
}

void PlayerPowersHandler::run() {
  vector<unsigned int> memorizedScrollsToShow;

  for(unsigned int i = 0; i < scrollsToReadFromPlayerMemory.size(); i++) {
    if(scrollsToReadFromPlayerMemory.at(i)->getDef().isScrollLearned) {
      memorizedScrollsToShow.push_back(i);
    }
  }

  const int NR_MEMORIZED_SPELLS = memorizedScrollsToShow.size();

  if(NR_MEMORIZED_SPELLS == 0) {
    eng->log->addMessage("I do not know any spells to invoke.");
  } else {
    MenuBrowser browser(NR_MEMORIZED_SPELLS, 0);

    eng->renderer->drawMapAndInterface();
    sf::Texture bgTexture = eng->renderer->getScreenTextureCopy();

    draw(browser, memorizedScrollsToShow, bgTexture);

    while(true) {
      const MenuAction_t action = eng->menuInputHandler->getAction(browser);
      switch(action) {
        case menuAction_browsed: {
          draw(browser, memorizedScrollsToShow, bgTexture);
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
          const unsigned int ELEMENT_SELECTED = browser.getPos().y;
          Scroll* const scroll = dynamic_cast<Scroll*>(scrollsToReadFromPlayerMemory.at(memorizedScrollsToShow.at(ELEMENT_SELECTED)));
          eng->log->clearLog();
          eng->renderer->drawMapAndInterface();
          scroll->read(true, eng);
          return;
        }
        break;
        default:
        {} break;
      }
    }
  }
}

void PlayerPowersHandler::draw(MenuBrowser& browser, const vector<unsigned int> memorizedScrollsToShow, const sf::Texture& bgTexture) {
  eng->renderer->clearWindow();
  eng->renderer->drawScreenSizedTexture(bgTexture);

  const unsigned int NR_OF_MEMORIZED = memorizedScrollsToShow.size();

  string endLetter = "a";
  endLetter[0] += static_cast<char>(NR_OF_MEMORIZED) - 1;

  const string label = "Choose a memorized power to evoke [a-" + endLetter + "] | Space/esc to exit";
  eng->renderer->drawText(label, renderArea_screen, 1, 1, clrWhiteHigh);

  int currentListPos = 0;

  for(unsigned int i = 0; i < NR_OF_MEMORIZED; i++) {
    const char CURRENT_KEY = 'a' + currentListPos;
    const sf::Color clr = browser.isPosAtKey(CURRENT_KEY) ? clrWhite : clrRedLight;
    Scroll* const scroll = scrollsToReadFromPlayerMemory.at(memorizedScrollsToShow.at(i));
    const string itemName = scroll->getRealTypeName();
    string str = "a";
    str[0] = CURRENT_KEY;
    str += ") " + itemName;

    eng->renderer->drawText(str, renderArea_mainScreen, 1, currentListPos, clr);

    string fill;
    fill.resize(0);
    const unsigned int FILL_SIZE = 28 - str.size();
    for(unsigned int ii = 0; ii < FILL_SIZE; ii++) {
      fill.push_back('.');
    }
    sf::Color fillClr = clrGray;
    fillClr.r /= 3;
    fillClr.g /= 3;
    fillClr.b /= 3;
    eng->renderer->drawText(fill, renderArea_mainScreen, 1 + str.size(), currentListPos, fillClr);
    const int x = 28;

    const int CHANCE_OF_SUCCESS = scroll->getChanceToCastFromMemory(eng);
    string s = intToString(CHANCE_OF_SUCCESS) + "% ";
    if(eng->playerBonusHandler->isBonusPicked(playerBonus_occultist)) {
      const int TURNS_PER_PERCENT = scroll->getDef().spellTurnsPerPercentCooldown;
      const int TURN = eng->gameTime->getTurn();
      const int TURNS_SINCE_LAST_TICK = TURN - ((TURN / TURNS_PER_PERCENT) * TURNS_PER_PERCENT);
      const int TURNS_LEFT = max(0, ((CAST_FROM_MEMORY_CHANCE_LIM - CHANCE_OF_SUCCESS) * TURNS_PER_PERCENT) - TURNS_SINCE_LAST_TICK);
      if(TURNS_LEFT > 0) {
        s += "(" + intToString(TURNS_LEFT) + " turns left)";
      }
    }

    eng->renderer->drawText(s, renderArea_mainScreen, x, currentListPos, clrWhite);

    currentListPos++;
  }

  eng->renderer->updateWindow();
}

Scroll* PlayerPowersHandler::getScrollAt(const unsigned int SCROLL_VECTOR_ELEMENT) const {
  return scrollsToReadFromPlayerMemory.at(SCROLL_VECTOR_ELEMENT);
}

