#include "PlayerPowersHandler.h"

#include "SDL.h"

#include "Engine.h"
#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Render.h"
#include "Inventory.h"
#include "ItemFactory.h"

void PlayerPowersHandler::run(const bool CAST_FROM_MEMORY) {
  vector<unsigned int> playerSlotsWithScroll;
  vector<unsigned int> memorizedScrollsToShow;

  vector<Item*>* const generalInv = eng->player->getInventory()->getGeneral();

  if(CAST_FROM_MEMORY == false) {
    for(unsigned int i = 0; i < generalInv->size(); i++) {
      if(generalInv->at(i)->getDef().isScroll) {
        playerSlotsWithScroll.push_back(i);
      }
    }
  } else {
    for(unsigned int i = 0; i < scrollsToReadFromPlayerMemory.size(); i++) {
      if(scrollsToReadFromPlayerMemory.at(i)->getDef().isScrollLearned) {
        memorizedScrollsToShow.push_back(i);
      }
    }
  }

  const int LIST_SIZE_MEMORY = memorizedScrollsToShow.size();
  const int LIST_SIZE_SCROLLS = playerSlotsWithScroll.size();

  if((CAST_FROM_MEMORY && LIST_SIZE_MEMORY == 0) || (!CAST_FROM_MEMORY && LIST_SIZE_SCROLLS == 0)) {
    if(CAST_FROM_MEMORY) {
      eng->log->addMessage("I do not know any spells to invoke.");
    } else {
      eng->log->addMessage("I do not possess any written powers to invoke.");
    }
  } else {
    MenuBrowser browser(CAST_FROM_MEMORY ? LIST_SIZE_MEMORY : LIST_SIZE_SCROLLS, 0);

    draw(browser, true, CAST_FROM_MEMORY, playerSlotsWithScroll, memorizedScrollsToShow);

    bool done = false;
    while(done == false) {
      const MenuAction_t action = eng->menuInputHandler->getAction(browser);
      switch(action) {
      case menuAction_browsed: {
        draw(browser, false, CAST_FROM_MEMORY, playerSlotsWithScroll, memorizedScrollsToShow);
      }
      break;
      case menuAction_canceled: {
        eng->log->clearLog();
        eng->renderer->drawMapAndInterface();
        done = true;
      }
      break;
      case menuAction_selected: {
        eng->log->clearLog();
        const unsigned int ELEMENT_SELECTED = browser.getPos().y;

        if(CAST_FROM_MEMORY) {
          const unsigned int ELEMENT_SELECTED_MEM = ELEMENT_SELECTED - playerSlotsWithScroll.size();
          Scroll* const scroll = dynamic_cast<Scroll*>(scrollsToReadFromPlayerMemory.at(memorizedScrollsToShow.at(ELEMENT_SELECTED_MEM)));
          eng->log->clearLog();
          eng->renderer->drawMapAndInterface();
          scroll->read(true, eng);
          return;
        } else {
          Scroll* const scroll = dynamic_cast<Scroll*>(generalInv->at(playerSlotsWithScroll.at(ELEMENT_SELECTED)));
          if(scroll->read(false, eng)) {
            eng->player->getInventory()->decreaseItemInGeneral(playerSlotsWithScroll.at(ELEMENT_SELECTED));
          }
          return;
        }
      }
      break;
      }
    }
  }
}

void PlayerPowersHandler::draw(MenuBrowser& browser, const bool DRAW_COMMAND_PROMPT, const bool CAST_FROM_MEMORY,
                               const vector<unsigned int> generalInventorySlotsToShow, const vector<unsigned int> memorizedScrollsToShow) {

  const unsigned int NR_OF_MEMORIZED = memorizedScrollsToShow.size();
  const unsigned int NR_OF_ITEMS = generalInventorySlotsToShow.size();

  string endLetter = "a";
  endLetter[0] += static_cast<char>(NR_OF_ITEMS + NR_OF_MEMORIZED) - 1;

  if(DRAW_COMMAND_PROMPT == true) {
    if(CAST_FROM_MEMORY) {
      eng->log->addMessage("Choose a memorized power to evoke", clrWhiteHigh);
    } else {
      eng->log->addMessage("Choose a written power to evoke", clrWhiteHigh);
    }
    eng->log->addMessage("[a-" + endLetter + "] [Space/esc] Exit", clrWhiteHigh);
  }

  const int LIST_HEIGHT = static_cast<int>(endLetter[0] - 'a') + 1;

  eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, 0, 0, MAP_X_CELLS, LIST_HEIGHT);

  int currentListPos = 0;
  for(unsigned int i = 0; i < NR_OF_ITEMS; i++) {
    const char CURRENT_KEY = 'a' + currentListPos;
    const SDL_Color clr = browser.isPosAtKey(CURRENT_KEY) ? clrWhite : clrRedLight;
    Item* const item = eng->player->getInventory()->getGeneral()->at(generalInventorySlotsToShow.at(i));
    const string itemName = eng->itemData->itemInterfaceName(item, true);
    string str = "a";
    str[0] = CURRENT_KEY;
    str += ") " + itemName;
    eng->renderer->drawText(str, renderArea_mainScreen, 1, currentListPos, clr);

//		string fill;
//		fill.resize(0);
//		const unsigned int FILL_SIZE = 65 - str.size();
//		for(unsigned int i = 0; i < FILL_SIZE; i++) {
//			fill.push_back('.');
//		}
//		eng->renderer->drawText(fill, renderArea_mainScreen, 1 + str.size(), currentListPos, clrGray);
//		const int x = 28;
//
//		const int CHANCE_OF_SUCCESS = dynamic_cast<Scroll*>(item)->getChanceToLearnOrCastFromMemory(false, eng);
//
//		const string info = "(" + intToString(CHANCE_OF_SUCCESS) + "% chance to memorize when cast)";
//		eng->renderer->drawText(info, renderArea_mainScreen, x, currentListPos, clrWhite);

    currentListPos++;
  }

  for(unsigned int i = 0; i < NR_OF_MEMORIZED; i++) {
    const char CURRENT_KEY = 'a' + currentListPos;
    const SDL_Color clr = browser.isPosAtKey(CURRENT_KEY) ? clrWhite : clrRedLight;
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
    eng->renderer->drawText(fill, renderArea_mainScreen, 1 + str.size(), currentListPos, clrGray);
    const int x = 28;

    const int CHANCE_OF_SUCCESS = scroll->getChanceToCastFromMemory(eng);
    string s = intToString(CHANCE_OF_SUCCESS) + "% ";
    const int TURNS_PER_PERCENT = scroll->getDef().spellTurnsPerPercentCooldown;
    const int TURN = eng->gameTime->getTurn();
    const int TURNS_SINCE_LAST_TICK = TURN - ((TURN / TURNS_PER_PERCENT ) * TURNS_PER_PERCENT );
    const int TURNS_LEFT = ((100 - CHANCE_OF_SUCCESS) * TURNS_PER_PERCENT) - TURNS_SINCE_LAST_TICK;
    s += "(" + intToString(TURNS_LEFT) + " turns left)";

    eng->renderer->drawText(s, renderArea_mainScreen, x, currentListPos, clrWhite);

    currentListPos++;
  }

  eng->renderer->flip();
}

PlayerPowersHandler::PlayerPowersHandler(Engine* engine) :
  eng(engine) {

  for(unsigned int i = 1; i < endOfItemDevNames; i++) {
    const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isScroll == true) {
      scrollsToReadFromPlayerMemory.push_back(dynamic_cast<Scroll*>(eng->itemFactory->spawnItem(static_cast<ItemDevNames_t>(i))));
    }
  }
}

PlayerPowersHandler::~PlayerPowersHandler() {
  for(unsigned int i = 0; i < scrollsToReadFromPlayerMemory.size(); i++) {
    delete scrollsToReadFromPlayerMemory.at(i);
  }
}
