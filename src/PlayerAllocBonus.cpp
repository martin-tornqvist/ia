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
#include "PlayerCreateCharacter.h"

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

void PlayerAllocBonus::draw(const vector<PlayerBonuses_t>& bonusesColumnOne,
                            const vector<PlayerBonuses_t>& bonusesColumnTwo,
                            const MenuBrowser& browser) const {
  eng->renderer->coverRenderArea(renderArea_screen);

  const unsigned int NR_BONUSES_COLUMN_ONE = bonusesColumnOne.size();
  const unsigned int NR_BONUSES_COLUMN_TWO = bonusesColumnTwo.size();

  const int X_COLUMN_ONE = 14;
  const int X_COLUMN_TWO = MAP_X_CELLS_HALF + 6;

  const int Y0_TITLE = Y0_CREATE_CHARACTER;

  eng->renderer->drawTextCentered("You have reached a new character level!", renderArea_screen,
                                  MAP_X_CELLS_HALF, Y0_TITLE - 1, clrWhite, clrBlack, true);

  eng->renderer->drawTextCentered("Which ability do you gain?", renderArea_screen,
                                  MAP_X_CELLS_HALF, Y0_TITLE, clrWhite, clrBlack, true);

  const coord browserPos = browser.getPos();

  //Draw bonuses
  const int Y0_BONUSES = Y0_TITLE + 2;
  int yPos = Y0_BONUSES;
  for(unsigned int i = 0; i < NR_BONUSES_COLUMN_ONE; i++) {
    const PlayerBonuses_t currentBonus = bonusesColumnOne.at(i);
    const string name = eng->playerBonusHandler->getBonusTitle(currentBonus);
    const bool IS_BONUS_MARKED = browserPos.x == 0 && browserPos.y == static_cast<int>(i);
    SDL_Color drwClr = IS_BONUS_MARKED ? clrNosferatuTealLgt : clrNosferatuTealDrk;
    eng->renderer->drawText(name, renderArea_screen, X_COLUMN_ONE, yPos, drwClr);
    yPos++;
  }
  yPos = Y0_BONUSES;
  for(unsigned int i = 0; i < NR_BONUSES_COLUMN_TWO; i++) {
    const PlayerBonuses_t currentBonus = bonusesColumnTwo.at(i);
    const string name = eng->playerBonusHandler->getBonusTitle(currentBonus);
    const bool IS_BONUS_MARKED = browserPos.x == 1 && browserPos.y == static_cast<int>(i);
    SDL_Color drwClr = IS_BONUS_MARKED ? clrNosferatuTealLgt : clrNosferatuTealDrk;
    eng->renderer->drawText(name, renderArea_screen, X_COLUMN_TWO, yPos, drwClr);
    yPos++;
  }

  //Draw description
  const int Y0_DESCR = Y0_BONUSES + NR_BONUSES_COLUMN_ONE + 2;
  yPos = Y0_DESCR;
  const PlayerBonuses_t markedBonus = browserPos.x == 0 ? bonusesColumnOne.at(browserPos.y) : bonusesColumnTwo.at(browserPos.y);
  string descr = eng->playerBonusHandler->getBonusDescription(markedBonus);
  const int MAX_WIDTH_DESCR = 50;
  vector<string> descrLines = eng->textFormatting->lineToLines("Effect(s): " + descr, MAX_WIDTH_DESCR);
  for(unsigned int i = 0; i < descrLines.size(); i++) {
    eng->renderer->drawText(descrLines.at(i), renderArea_screen, X_COLUMN_ONE, yPos, clrNosferatuTealDrk);
    yPos++;
  }
  yPos++;

  yPos = max(Y0_DESCR + 3, yPos);

  vector<PlayerBonuses_t> prereqsForCurrentBonus = eng->playerBonusHandler->getBonusPrereqs(markedBonus);
  const unsigned int NR_PREREQS = prereqsForCurrentBonus.size();
  if(NR_PREREQS > 0) {
    string prereqStr = "This ability had the following prerequisite(s): ";
    for(unsigned int i = 0; i < NR_PREREQS; i++) {
      const PlayerBonuses_t prereq = prereqsForCurrentBonus.at(i);
      const string prereqTitle = eng->playerBonusHandler->getBonusTitle(prereq);
      if(i == 0) {
        prereqStr += "\"" + prereqTitle + "\"";
      } else {
        prereqStr += ", \"" + prereqTitle + "\"";
      }
    }
    vector<string> prereqLines = eng->textFormatting->lineToLines(prereqStr, MAX_WIDTH_DESCR);
    for(unsigned int i = 0; i < prereqLines.size(); i++) {
      eng->renderer->drawText(prereqLines.at(i), renderArea_screen, X_COLUMN_ONE, yPos, clrNosferatuTealDrk);
      yPos++;
    }
  }

  eng->renderer->updateScreen();
}


