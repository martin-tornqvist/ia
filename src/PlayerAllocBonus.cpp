#include "PlayerAllocBonus.h"

#include <math.h>
#include <iostream>

#include "Engine.h"
#include "Colors.h"
#include "Log.h"
#include "Render.h"
#include "DungeonMaster.h"

void PlayerAllocBonus::run() {
  readKeys();
  eng->log->drawLog();
  eng->renderer->drawMapAndInterface();
}

void PlayerAllocBonus::draw(const int PICKS_LEFT, const unsigned int CURRENT_POS, const unsigned int TOP_BONUS) const {
  (void)PICKS_LEFT;

  eng->renderer->clearRenderArea(renderArea_screen);

  int xPos = 1;
  int yPos = 1;
  const int X_POS_GROUP_TITLE = 1;

  //	eng->renderer->drawText("--- Choose ability to improve (" + intToString(PICKS_LEFT) + ") ---", renderArea_screen, xPos, yPos, clrWhite);
  eng->renderer->drawText("--- Choose ability to improve ---", renderArea_screen, xPos, yPos, clrWhite);

  const unsigned int NR_OF_BONUSES = eng->playerBonusHandler->getNrOfBonuses();

  int longestTitleLength = 0;
  int longestGroupTitleLength = 0;

  for(unsigned int i = 0; i < NR_OF_BONUSES; i++) {
    const int CURRENT_LENGTH = static_cast<int>(eng->playerBonusHandler->getBonusTitleAt(i).length() + 2);
    longestTitleLength = max(longestTitleLength, CURRENT_LENGTH);
  }

  for(unsigned int i = 0; i < NR_OF_BONUSES; i++) {
    const int CURRENT_LENGTH = static_cast<int>(eng->playerBonusHandler->getBonusGroupTitleAt(i).length() + 2);
    longestGroupTitleLength = max(longestGroupTitleLength, CURRENT_LENGTH);
  }

  //	if(TOP_BONUS > 0) {
  //		eng->renderer->drawText("^^^ MORE ^^^", 62, 1, clrGreenLight);
  //	} else {
  //		eng->renderer->drawText("vvv MORE vvv", 62, ELEMENTS_ON_SCREEN + 3, clrGreenLight);
  //	}

  string currentGroupTitle = "";
  yPos = 2;
  for(unsigned int i = 0; i < ELEMENTS_ON_SCREEN && i < NR_OF_BONUSES; i++) {
    xPos = X_POS_GROUP_TITLE + longestGroupTitleLength + 1;

    const unsigned int CURRENT_ELEMENT = TOP_BONUS + i;

    //Draw group title, if new group
    if(eng->playerBonusHandler->getBonusGroupTitleAt(CURRENT_ELEMENT) != currentGroupTitle) {
      yPos++;
      currentGroupTitle = eng->playerBonusHandler->getBonusGroupTitleAt(CURRENT_ELEMENT);
      string fill(longestGroupTitleLength - static_cast<int>(currentGroupTitle.size()), '.');
      string s = currentGroupTitle + fill;
      eng->renderer->drawText(s, renderArea_screen, X_POS_GROUP_TITLE, yPos, clrMagenta);
    }

    //Draw bonus title
    string s = eng->playerBonusHandler->getBonusTitleAt(CURRENT_ELEMENT);
    string fill(longestTitleLength - static_cast<int>(s.size()) + 2, '.');
    s += fill;
    const bool IS_PICKABLE = eng->playerBonusHandler->isBonusPickableAt(CURRENT_ELEMENT);
    SDL_Color drwClr = CURRENT_ELEMENT == CURRENT_POS ? (IS_PICKABLE ? clrRedLight : clrWhite) : (IS_PICKABLE ? clrRed : clrGray);

    eng->renderer->drawText(s, renderArea_screen, xPos, yPos, drwClr);

    xPos += s.size() + 1;

    //Draw rank
    const int RANK = eng->playerBonusHandler->getBonusRankAt(CURRENT_ELEMENT);
    const int RANK_LIMIT = eng->playerBonusHandler->getBonusRankLimitAt(CURRENT_ELEMENT);
    int xPosTemp = xPos;
    for(int ii = 1; ii <= RANK_LIMIT; ii++) {
      s = RANK >= ii ? "*" : "-";
      drwClr = RANK >= ii ? clrRedLight : clrBlue;
      eng->renderer->drawText(s, renderArea_screen, xPosTemp, yPos, drwClr);
      xPosTemp++;
    }

    xPos = 1;

    yPos++;
  }

  yPos += 2;

  int& yPosRankDescr = yPos;//yPos + 4;

  //Draw description
  const vector<string>& description = eng->playerBonusHandler->getBonusDescriptionGeneralAt(CURRENT_POS);
  for(unsigned int i_descr = 0; i_descr < description.size(); i_descr++) {
    string s = description.at(i_descr);
    eng->renderer->drawText(s, renderArea_screen, xPos, yPos, clrWhite);
    yPos++;
  }

  yPos++;

  const vector< vector<string> >& rankDescriptions = eng->playerBonusHandler->getBonusDescriptionRanksAt(CURRENT_POS);



  bool isRankBonusTitleDrawn = false;

  for(unsigned int ii = 0; ii < rankDescriptions.size(); ii++) {
    const vector<string>& currentRankDescr = rankDescriptions.at(ii);
    for(unsigned int iii = 0; iii < currentRankDescr.size(); iii++) {
      const string s = currentRankDescr.at(iii);

      const string compareSizeStr = "Rank x: ";
      const bool IS_CURRENT_RANK_DESCRIBED = s.size() > compareSizeStr.size();

      if(IS_CURRENT_RANK_DESCRIBED) {

        if(isRankBonusTitleDrawn == false) {
          eng->renderer->drawText("Additional bonuses:", renderArea_screen, xPos, yPos, clrWhite);
          yPos++;
          isRankBonusTitleDrawn = true;
        }

        const bool HAS_RANK = eng->playerBonusHandler->getBonusRankAt(CURRENT_POS) > static_cast<int>(ii);
        const SDL_Color drwClr = HAS_RANK ? clrGreenLight : clrWhite;
        eng->renderer->drawText(s, renderArea_screen, xPos, yPosRankDescr, drwClr);

        yPosRankDescr++;
      }
    }
  }

  eng->renderer->flip();
}

void PlayerAllocBonus::browseUp(unsigned int& currentPos, unsigned int& topElement, const unsigned int NR_OF_BONUSES_DEFINED) {
  currentPos = currentPos == 0 ? NR_OF_BONUSES_DEFINED - 1 : currentPos - 1;
  setTopElement(currentPos, topElement, NR_OF_BONUSES_DEFINED);
}

void PlayerAllocBonus::browseDown(unsigned int& currentPos, unsigned int& topElement, const unsigned int NR_OF_BONUSES_DEFINED) {
  currentPos = currentPos == NR_OF_BONUSES_DEFINED - 1 ? 0 : currentPos + 1;
  setTopElement(currentPos, topElement, NR_OF_BONUSES_DEFINED);
}

void PlayerAllocBonus::setTopElement(unsigned int& currentPos, unsigned int& topElement, const unsigned int NR_OF_BONUSES_DEFINED) {
  if(currentPos == 0) {
    topElement = 0;
    return;
  }

  if(currentPos == NR_OF_BONUSES_DEFINED - 1) {
    topElement = static_cast<unsigned int>(max(0, static_cast<int>(NR_OF_BONUSES_DEFINED) - static_cast<int>(ELEMENTS_ON_SCREEN)));
    return;
  }

  if(currentPos >= topElement + ELEMENTS_ON_SCREEN) {
    topElement++;
    return;
  }
  if(currentPos < topElement) {
    topElement--;
  }
}

void PlayerAllocBonus::readKeys() {
  const unsigned int NR_OF_BONUSES_DEFINED = eng->playerBonusHandler->getNrOfBonuses();
  unsigned int topBonus = 0;
  unsigned int currentPos = 0;

  //	const int LVL = eng->dungeonMaster->getLevel();
  int nrOfPicksLeft = 1; //((LVL + 1) % 2 == 0) ? 2 : 1;

  eng->playerBonusHandler->setAllToUnpicked();

  draw(nrOfPicksLeft, currentPos, topBonus);

  bool done = false;
  while(done == false) {
    if(eng->playerBonusHandler->canAnyBonusBePicked() == false) {
      done = true;
      return;
    }

    while(SDL_PollEvent(&m_event)) {
      switch(m_event.type) {
      case SDL_KEYDOWN: {
        Uint16 key = static_cast<Uint16>(m_event.key.keysym.sym);

        if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
          if(eng->playerBonusHandler->isBonusPickableAt(currentPos)) {
            nrOfPicksLeft--;
            eng->playerBonusHandler->increaseBonusAt(currentPos, eng);
            if(nrOfPicksLeft > 0) {
              draw(nrOfPicksLeft, currentPos, topBonus);
            } else {
              return;
            }
          }
        }

        if(key == SDLK_UP || key == SDLK_KP8 || key == SDLK_8) {
          browseUp(currentPos, topBonus, NR_OF_BONUSES_DEFINED);
          draw(nrOfPicksLeft, currentPos, topBonus);
        }

        if(key == SDLK_DOWN || key == SDLK_KP2 || key == SDLK_2) {
          browseDown(currentPos, topBonus, NR_OF_BONUSES_DEFINED);
          draw(nrOfPicksLeft, currentPos, topBonus);
        }
      }
      default: {
      }
      break;
      }
    }
    SDL_Delay(1);
  }
}

