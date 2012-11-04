#include "PlayerAllocBonus.h"

#include <math.h>
#include <iostream>

#include "Engine.h"
#include "Colors.h"
#include "Log.h"
#include "Render.h"
#include "DungeonMaster.h"
#include "MenuInputHandler.h"

void PlayerAllocBonus::run() {
  const vector<PlayerBonuses_t> bonusesToChooseFrom = eng->playerBonusHandler->getBonusChoices();

  MenuBrowser browser(bonusesToChooseFrom.size(), 0);
  draw(bonusesToChooseFrom, browser);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      draw(bonusesToChooseFrom, browser);
    } break;

    case menuAction_canceled: {
    } break;

    case menuAction_selected: {
      eng->playerBonusHandler->pickBonus(bonusesToChooseFrom.at(browser.getPos().y));
      eng->log->drawLog();
      eng->renderer->drawMapAndInterface();
      return;
    } break;
    }
  }
}

void PlayerAllocBonus::draw(const vector<PlayerBonuses_t>& bonusesToChooseFrom, const MenuBrowser& browser) const {
  eng->renderer->clearRenderArea(renderArea_screen);

  const int X_POS_ABILITY = 1;
  const int X_POS_DESCR = X_POS_ABILITY + 3;
  const int X_POS_TOP = 1;

  int yPos = eng->config->MAINSCREEN_Y_CELLS_OFFSET;
  eng->renderer->drawText("--- Choose new ability ---", renderArea_screen, X_POS_TOP, yPos, clrWhite);
  const unsigned int NR_OF_BONUSES = bonusesToChooseFrom.size();

  yPos += 2;
  for(unsigned int i = 0; i < NR_OF_BONUSES; i++) {
    const PlayerBonuses_t currentBonus = bonusesToChooseFrom.at(i);

    //Draw bonus title
    string s = "   ";
    s[0] = 'a' + i;
    s[1] = ')';
    s += eng->playerBonusHandler->getBonusTitle(currentBonus);
    SDL_Color drwClr = static_cast<unsigned int>(browser.getPos().y) == i ? clrWhite : clrRedLight;
    eng->renderer->drawText(s, renderArea_screen, X_POS_ABILITY, yPos, drwClr);
    yPos++;

    //Draw description
    s = eng->playerBonusHandler->getBonusDescription(currentBonus);
    eng->renderer->drawText(s, renderArea_screen, X_POS_DESCR, yPos, clrRed);

    yPos += 2;
  }

  eng->renderer->flip();
}


