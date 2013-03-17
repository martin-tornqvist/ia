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
  vector<PlayerBonuses_t> bonuses = eng->playerBonusHandler->getBonusChoices();

  if(bonuses.empty() == false) {

    const unsigned int NR_BONUSES_TOT = bonuses.size();

    const unsigned int NR_BONUSES_COLUMN_TWO = NR_BONUSES_TOT / 2;
    const unsigned int NR_BONUSES_COLUMN_ONE = NR_BONUSES_TOT - NR_BONUSES_COLUMN_TWO;

    vector<PlayerBonuses_t> bonusesColumnOne;
    vector<PlayerBonuses_t> bonusesColumnTwo;

    for(unsigned int i = 0; i < NR_BONUSES_TOT; i++) {
      const PlayerBonuses_t bonus = bonuses.at(i);
      if(i < NR_BONUSES_COLUMN_ONE) {
        bonusesColumnOne.push_back(bonus);
      } else {
        bonusesColumnTwo.push_back(bonus);
      }
    }

    MenuBrowser browser(bonusesColumnOne.size(), bonusesColumnTwo.size());
    draw(bonusesColumnOne, bonusesColumnTwo, browser);

    while(true) {
      const MenuAction_t action = eng->menuInputHandler->getAction(browser);
      switch(action) {
      case menuAction_browsed: {
        draw(bonusesColumnOne, bonusesColumnTwo, browser);
      }
      break;

      case menuAction_canceled: {
      } break;

      case menuAction_selected: {
        const coord browserPos = browser.getPos();
        if(browserPos.x == 0) {
          eng->playerBonusHandler->pickBonus(bonusesColumnOne.at(browser.getPos().y));
        } else {
          eng->playerBonusHandler->pickBonus(bonusesColumnTwo.at(browser.getPos().y));
        }
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

void PlayerAllocBonus::draw(const vector<PlayerBonuses_t>& bonusesColumnOne, const vector<PlayerBonuses_t>& bonusesColumnTwo,
                            const MenuBrowser& browser) const {
  eng->renderer->coverRenderArea(renderArea_screen);

  const unsigned int NR_BONUSES_COLUMN_ONE = bonusesColumnOne.size();
  const unsigned int NR_BONUSES_COLUMN_TWO = bonusesColumnTwo.size();

//  unsigned int sizeOfLongestNameInFirstCol = 0;
//  for(unsigned int i = 0; i < NR_BONUSES_COLUMN_ONE; i++) {
//    const string& name = eng->playerBonusHandler->getBonusTitle(bonusesColumnOne.at(i));
//    if(name.size() > sizeOfLongestNameInFirstCol) {
//      sizeOfLongestNameInFirstCol = name.size();
//    }
//  }

//  const int COLUMNS_W_FROM_CENTER = 3;
//  const int X_COLUMN_ONE = MAP_X_CELLS_HALF - COLUMNS_W_FROM_CENTER - sizeOfLongestNameInFirstCol + 1;
//  const int X_COLUMN_TWO = MAP_X_CELLS_HALF + COLUMNS_W_FROM_CENTER;

  const int X_COLUMN_ONE = 14;
  const int X_COLUMN_TWO = MAP_X_CELLS_HALF + 6;

  const int Y0_BONUSES = MAP_Y_CELLS_HALF - (NR_BONUSES_COLUMN_ONE / 2);

  eng->renderer->drawTextCentered("Choose new ability", renderArea_screen, MAP_X_CELLS_HALF, Y0_BONUSES - 2, clrWhite, true);

  const coord browserPos = browser.getPos();

  //Draw bonuses
  int yPos = Y0_BONUSES;
  for(unsigned int i = 0; i < NR_BONUSES_COLUMN_ONE; i++) {
    const PlayerBonuses_t currentBonus = bonusesColumnOne.at(i);
    const string name = eng->playerBonusHandler->getBonusTitle(currentBonus);
    const bool IS_BONUS_MARKED = browserPos.x == 0 && browserPos.y == static_cast<int>(i);
    sf::Color drwClr = IS_BONUS_MARKED ? clrWhite : clrRedLight;
    eng->renderer->drawText(name, renderArea_screen, X_COLUMN_ONE, yPos, drwClr);
    yPos++;
  }
  yPos = Y0_BONUSES;
  for(unsigned int i = 0; i < NR_BONUSES_COLUMN_TWO; i++) {
    const PlayerBonuses_t currentBonus = bonusesColumnTwo.at(i);
    const string name = eng->playerBonusHandler->getBonusTitle(currentBonus);
    const bool IS_BONUS_MARKED = browserPos.x == 1 && browserPos.y == static_cast<int>(i);
    sf::Color drwClr = IS_BONUS_MARKED ? clrWhite : clrRedLight;
    eng->renderer->drawText(name, renderArea_screen, X_COLUMN_TWO, yPos, drwClr);
    yPos++;
  }

  //Draw description
  yPos = Y0_BONUSES + NR_BONUSES_COLUMN_ONE + 2;
  const PlayerBonuses_t markedBonus = browserPos.x == 0 ? bonusesColumnOne.at(browserPos.y) : bonusesColumnTwo.at(browserPos.y);
  string descr = eng->playerBonusHandler->getBonusDescription(markedBonus);
  vector<string> descrLines = eng->textFormatting->lineToLines(descr, 50);
//  const int X_POS_DESCR_LEFT_AFTER_FIRST = MAP_X_CELLS_HALF - descrLines.at(0).size() / 2;
  for(unsigned int iDescr = 0; iDescr < descrLines.size(); iDescr++) {
    eng->renderer->drawText(descrLines.at(iDescr), renderArea_screen, X_COLUMN_ONE, yPos, clrRed);
    yPos++;
  }

  eng->renderer->updateWindow();
}


