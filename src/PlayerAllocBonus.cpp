#include "PlayerAllocBonus.h"

#include <math.h>
#include <iostream>

#include "Engine.h"
#include "Colors.h"
#include "Log.h"
#include "Render.h"
#include "DungeonMaster.h"
#include "MenuInputHandler.h"
#include "TextFormatting.h"

void PlayerAllocBonus::run() {
  const vector<PlayerBonuses_t> bonusesToChooseFrom = eng->playerBonusHandler->getBonusChoices();

  if(bonusesToChooseFrom.empty() == false) {
    MenuBrowser browser(bonusesToChooseFrom.size(), 0);
    draw(bonusesToChooseFrom, browser);

    while(true) {
      const MenuAction_t action = eng->menuInputHandler->getAction(browser);
      switch(action) {
      case menuAction_browsed: {
        draw(bonusesToChooseFrom, browser);
      }
      break;

      case menuAction_canceled: {
      } break;

      case menuAction_selected: {
        eng->playerBonusHandler->pickBonus(bonusesToChooseFrom.at(browser.getPos().y));
        eng->log->drawLog();
        eng->renderer->drawMapAndInterface();
        return;
      }
      break;

      case menuAction_selectedWithShift:
      {} break;

      }
    }
  }
}

void PlayerAllocBonus::draw(const vector<PlayerBonuses_t>& bonusesToChooseFrom, const MenuBrowser& browser) const {
  eng->renderer->coverRenderArea(renderArea_screen);

  int yPos = 8;
  eng->renderer->drawTextCentered("Choose new ability", renderArea_screen, MAP_X_CELLS_HALF, yPos, clrWhite);
  const unsigned int NR_OF_BONUSES = bonusesToChooseFrom.size();

  yPos += 2;

  //Draw bonuses
  for(unsigned int i = 0; i < NR_OF_BONUSES; i++) {
    const PlayerBonuses_t currentBonus = bonusesToChooseFrom.at(i);
    string s = eng->playerBonusHandler->getBonusTitle(currentBonus);
    const bool IS_MARKED_BONUS = static_cast<unsigned int>(browser.getPos().y) == i;
    sf::Color drwClr = IS_MARKED_BONUS ? clrWhite : clrRedLight;
    eng->renderer->drawTextCentered(s, renderArea_screen, MAP_X_CELLS_HALF, yPos, drwClr);
    yPos++;
  }

  yPos++;

  //Draw description
  string descr = eng->playerBonusHandler->getBonusDescription(bonusesToChooseFrom.at(browser.getPos().y));
  vector<string> descrLines = eng->textFormatting->lineToLines(descr, 50);
  const int X_POS_DESCR_LEFT_AFTER_FIRST = MAP_X_CELLS_HALF - descrLines.at(0).size() / 2;
  for(unsigned int iDescr = 0; iDescr < descrLines.size(); iDescr++) {
    if(iDescr == 0) {
      eng->renderer->drawTextCentered(descrLines.at(iDescr), renderArea_screen, MAP_X_CELLS_HALF, yPos, clrRed, false);
    } else {
      eng->renderer->drawText(descrLines.at(iDescr), renderArea_screen, X_POS_DESCR_LEFT_AFTER_FIRST, yPos, clrRed);
    }
    yPos++;
  }

  eng->renderer->updateWindow();
}


