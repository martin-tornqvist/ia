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
  vector<PlayerBon_t> bonuses = eng->playerBonHandler->getBonusChoices();

  if(bonuses.empty() == false) {

    const unsigned int NR_BONUSES_TOT = bonuses.size();

    const unsigned int NR_BONUSES_COL_TWO = NR_BONUSES_TOT / 2;
    const unsigned int NR_BONUSES_COL_ONE = NR_BONUSES_TOT - NR_BONUSES_COL_TWO;

    vector<PlayerBon_t> bonusesColOne;
    vector<PlayerBon_t> bonusesColTwo;

    for(unsigned int i = 0; i < NR_BONUSES_TOT; i++) {
      const PlayerBon_t bonus = bonuses.at(i);
      if(i < NR_BONUSES_COL_ONE) {
        bonusesColOne.push_back(bonus);
      } else {
        bonusesColTwo.push_back(bonus);
      }
    }

    MenuBrowser browser(bonusesColOne.size(), bonusesColTwo.size());
    draw(bonusesColOne, bonusesColTwo, browser);

    while(true) {
      const MenuAction_t action = eng->menuInputHandler->getAction(browser);
      switch(action) {
        case menuAction_browsed: {
          draw(bonusesColOne, bonusesColTwo, browser);
        }
        break;

        case menuAction_canceled: {
        } break;

        case menuAction_selected: {
          const Pos browserPos = browser.getPos();
          if(browserPos.x == 0) {
            eng->playerBonHandler->pickBonus(
              bonusesColOne.at(browser.getPos().y));
          } else {
            eng->playerBonHandler->pickBonus(
              bonusesColTwo.at(browser.getPos().y));
          }
          eng->log->clearLog();
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

void PlayerAllocBonus::draw(const vector<PlayerBon_t>& bonusesColOne,
                            const vector<PlayerBon_t>& bonusesColTwo,
                            const MenuBrowser& browser) const {
  eng->renderer->coverPanel(panel_screen);

  const unsigned int NR_BONUSES_COL_ONE = bonusesColOne.size();
  const unsigned int NR_BONUSES_COL_TWO = bonusesColTwo.size();

  const int X_COL_ONE = 14;
  const int X_COL_TWO = MAP_X_CELLS_HALF + 6;

  const int Y0_TITLE = Y0_CREATE_CHARACTER;

  eng->renderer->drawTextCentered(
    "You have reached a new character level!", panel_screen,
    Pos(MAP_X_CELLS_HALF, Y0_TITLE - 1), clrWhite, clrBlack, true);

  eng->renderer->drawTextCentered(
    "Which ability do you gain?", panel_screen,
    Pos(MAP_X_CELLS_HALF, Y0_TITLE), clrWhite, clrBlack, true);

  const Pos browserPos = browser.getPos();

  //Draw bonuses
  const int Y0_BONUSES = Y0_TITLE + 2;
  int yPos = Y0_BONUSES;
  for(unsigned int i = 0; i < NR_BONUSES_COL_ONE; i++) {
    const PlayerBon_t currentBonus = bonusesColOne.at(i);
    const string name = eng->playerBonHandler->getBonusTitle(currentBonus);
    const bool IS_BONUS_MARKED = browserPos.x == 0 && browserPos.y == int(i);
    SDL_Color drwClr =
      IS_BONUS_MARKED ? clrNosferatuTealLgt : clrNosferatuTealDrk;
    eng->renderer->drawText(name, panel_screen, Pos(X_COL_ONE, yPos), drwClr);
    yPos++;
  }
  yPos = Y0_BONUSES;
  for(unsigned int i = 0; i < NR_BONUSES_COL_TWO; i++) {
    const PlayerBon_t currentBonus = bonusesColTwo.at(i);
    const string name = eng->playerBonHandler->getBonusTitle(currentBonus);
    const bool IS_BONUS_MARKED = browserPos.x == 1 && browserPos.y == int(i);
    SDL_Color drwClr =
      IS_BONUS_MARKED ? clrNosferatuTealLgt : clrNosferatuTealDrk;
    eng->renderer->drawText(name, panel_screen, Pos(X_COL_TWO, yPos), drwClr);
    yPos++;
  }

  //Draw description
  const int Y0_DESCR = Y0_BONUSES + NR_BONUSES_COL_ONE + 2;
  yPos = Y0_DESCR;
  const PlayerBon_t markedBonus =
    browserPos.x == 0 ? bonusesColOne.at(browserPos.y) :
    bonusesColTwo.at(browserPos.y);
  string descr = eng->playerBonHandler->getBonusDescription(markedBonus);
  const int MAX_WIDTH_DESCR = 50;
  vector<string> descrLines =
    eng->textFormatting->lineToLines("Effect(s): " + descr, MAX_WIDTH_DESCR);
  for(unsigned int i = 0; i < descrLines.size(); i++) {
    eng->renderer->drawText(descrLines.at(i), panel_screen,
                            Pos(X_COL_ONE, yPos), clrGray);
    yPos++;
  }
  yPos++;

  yPos = max(Y0_DESCR + 3, yPos);

  //Prerequisites
  vector<PlayerBon_t> prereqsForCurrentBonus =
    eng->playerBonHandler->getBonusPrereqs(markedBonus);
  const unsigned int NR_PREREQS = prereqsForCurrentBonus.size();
  if(NR_PREREQS > 0) {
    string prereqStr = "This ability had the following prerequisite(s): ";
    for(unsigned int i = 0; i < NR_PREREQS; i++) {
      const PlayerBon_t prereq = prereqsForCurrentBonus.at(i);
      const string prereqTitle = eng->playerBonHandler->getBonusTitle(prereq);
      if(i == 0) {
        prereqStr += "\"" + prereqTitle + "\"";
      } else {
        prereqStr += ", \"" + prereqTitle + "\"";
      }
    }
    vector<string> prereqLines =
      eng->textFormatting->lineToLines(prereqStr, MAX_WIDTH_DESCR);
    for(unsigned int i = 0; i < prereqLines.size(); i++) {
      eng->renderer->drawText(prereqLines.at(i), panel_screen,
                              Pos(X_COL_ONE, yPos), clrGray);
      yPos++;
    }
    yPos++;
  }

  //Previously picked bonuses
  string pickedBonusesLine = "";
  eng->playerBonHandler->getAllPickedBonusTitlesLine(pickedBonusesLine);
  if(pickedBonusesLine != "") {
    pickedBonusesLine = "Abilities previously gained: " + pickedBonusesLine;
    vector<string> pickedBonusesLines =
      eng->textFormatting->lineToLines(pickedBonusesLine, MAX_WIDTH_DESCR);
    for(unsigned int i = 0; i < pickedBonusesLines.size(); i++) {
      eng->renderer->drawText(pickedBonusesLines.at(i), panel_screen,
                              Pos(X_COL_ONE, yPos), clrGray);
      yPos++;
    }
  }

  eng->renderer->updateScreen();
}


