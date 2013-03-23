#include "Popup.h"

#include "Engine.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Log.h"
#include "Query.h"
#include "ConstDungeonSettings.h"
#include "MenuBrowser.h"
#include "MenuInputHandler.h"

const int TEXT_AREA_WIDTH = 38;
const int TEXT_AREA_X0 = MAP_X_CELLS_HALF - (TEXT_AREA_WIDTH) / 2;

int Popup::printBoxAndReturnTitleYPos(const int TEXT_AREA_HEIGHT) const {
  const int BOX_WIDTH = TEXT_AREA_WIDTH + 2;
  const int BOX_HEIGHT = TEXT_AREA_HEIGHT + 2;

  const int X0 = TEXT_AREA_X0 - 1;

  const int Y_OFFSET = -1;

  const int Y0 = MAP_Y_CELLS_HALF - BOX_HEIGHT / 2 + Y_OFFSET;
  const int X1 = X0 + BOX_WIDTH - 1;
  const int Y1 = Y0 + BOX_HEIGHT - 1;

  eng->renderer->coverArea(renderArea_mainScreen, X0, Y0, BOX_WIDTH, BOX_HEIGHT);

  const bool& USE_TILE_SET = eng->config->USE_TILE_SET;

  const sf::Color clrBox = clrGray;

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(x == X0 || x == X1) {
        if(y == Y0 || y == Y1) {
          if(USE_TILE_SET) {
            if(x == X0 && y == Y0) {
              eng->renderer->drawTileInMap(tile_popupCornerTopLeft, x, y, clrBox, false, clrBlack);
            } else if(x == X1 && y == Y0) {
              eng->renderer->drawTileInMap(tile_popupCornerTopRight, x, y, clrBox, false, clrBlack);
            } else if(x == X0 && y == Y1) {
              eng->renderer->drawTileInMap(tile_popupCornerBottomLeft, x, y, clrBox, false, clrBlack);
            } else {
              eng->renderer->drawTileInMap(tile_popupCornerBottomRight, x, y, clrBox, false, clrBlack);
            }
          } else {
            eng->renderer->drawCharacter('#', renderArea_mainScreen, x, y, clrBox);
          }
        } else {
          if(USE_TILE_SET) {
            eng->renderer->drawTileInMap(tile_popupVerticalBar, x, y, clrBox, false, clrBlack);
          } else {
            eng->renderer->drawCharacter('|', renderArea_mainScreen, x, y, clrBox);
          }
        }
      } else {
        if(y == Y0 || y == Y1) {
          if(USE_TILE_SET) {
            eng->renderer->drawTileInMap(tile_popupHorizontalBar, x, y, clrBox, false, clrBlack);
          } else {
            eng->renderer->drawCharacter('=', renderArea_mainScreen, x, y, clrBox);
          }
        }
      }
    }
  }

  return Y0 + 1;
}

void Popup::showMessage(const string& message, const bool DRAW_MAP_AND_INTERFACE, const string title) const {
  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface(false);
  }

  vector<string> lines = eng->textFormatting->lineToLines(message, TEXT_AREA_WIDTH);
  const int TEXT_AREA_HEIGHT =  static_cast<int>(lines.size()) + 3; //Title + text + blank + label

  const int TITLE_Y_POS = printBoxAndReturnTitleYPos(TEXT_AREA_HEIGHT);

  int yPos = TITLE_Y_POS;

  if(title != "") {
    eng->renderer->drawTextCentered(title, renderArea_mainScreen, MAP_X_CELLS_HALF, TITLE_Y_POS, clrCyanLight, true);
  }

  for(unsigned int i = 0; i < lines.size(); i++) {
    yPos++;
    eng->renderer->drawText(lines.at(i), renderArea_mainScreen, TEXT_AREA_X0, yPos, clrRedLight);
    eng->log->addLineToHistory(lines.at(i));
  }
  yPos += 2;

  eng->renderer->drawTextCentered("space/esc to close", renderArea_mainScreen, MAP_X_CELLS_HALF, yPos, clrWhiteHigh);

  eng->renderer->updateWindow();

  eng->query->waitForEscOrSpace();

  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface();
  }
}

unsigned int Popup::showMultiChoiceMessage(const string& message, const bool SHOW_MESSAGE_CENTERED,
    const bool DRAW_MAP_AND_INTERFACE, const vector<string>& choices, const string title) const {

  vector<string> lines = eng->textFormatting->lineToLines(message, TEXT_AREA_WIDTH);
  const int TEXT_HEIGHT = static_cast<int>(lines.size());
  const int NR_CHOICES = static_cast<int>(choices.size());

  const int TEXT_AREA_HEIGHT = TEXT_HEIGHT + NR_CHOICES + 3; //Title + text + blank + choices + blank

  MenuBrowser browser(NR_CHOICES, 0);

  multiChoiceMessageDrawingHelper(lines, SHOW_MESSAGE_CENTERED, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y, TEXT_AREA_HEIGHT, title);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);

    switch(action) {
    case menuAction_browsed: {
      multiChoiceMessageDrawingHelper(lines, SHOW_MESSAGE_CENTERED, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y, TEXT_AREA_HEIGHT, title);
    }
    break;

    case menuAction_canceled: {
    }
    break;

    case menuAction_selected: {
      if(DRAW_MAP_AND_INTERFACE) {
        eng->renderer->drawMapAndInterface();
      }
      return browser.getPos().y;
    }
    break;

    case menuAction_selectedWithShift: {
    }
    break;
    }
  }
}

void Popup::multiChoiceMessageDrawingHelper(const vector<string>& lines, const bool SHOW_MESSAGE_CENTERED,
    const vector<string>& choices, const bool DRAW_MAP_AND_INTERFACE, const unsigned int currentChoice,
    const int TEXT_AREA_HEIGHT, const string title) const {

  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface(false);
  }

  const int TITLE_Y_POS = printBoxAndReturnTitleYPos(TEXT_AREA_HEIGHT);

  int yPos = TITLE_Y_POS;

  if(title != "") {
    eng->renderer->drawTextCentered(title, renderArea_mainScreen, MAP_X_CELLS_HALF, TITLE_Y_POS, clrCyanLight, true);
  }
  for(unsigned int i = 0; i < lines.size(); i++) {
    yPos++;
    if(SHOW_MESSAGE_CENTERED) {
      eng->renderer->drawTextCentered(lines.at(i), renderArea_mainScreen, MAP_X_CELLS_HALF, yPos, clrRedLight, true);
    } else {
      eng->renderer->drawText(lines.at(i), renderArea_mainScreen, TEXT_AREA_X0, yPos, clrRedLight);
    }
    eng->log->addLineToHistory(lines.at(i));
  }
  yPos++;
  for(unsigned int i = 0; i < choices.size(); i++) {
    yPos++;
    sf::Color clr = i == currentChoice ? clrWhiteHigh : clrRedLight;
    eng->renderer->drawTextCentered(choices.at(i), renderArea_mainScreen, MAP_X_CELLS_HALF, yPos, clr, true);
    eng->log->addLineToHistory(choices.at(i));
  }
  eng->renderer->updateWindow();
}

