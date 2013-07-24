#include "Popup.h"

#include "Engine.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Log.h"
#include "Query.h"
#include "CommonTypes.h"
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

  eng->renderer->coverArea(panel_map, Pos(X0, Y0), Pos(BOX_WIDTH, BOX_HEIGHT));
  eng->renderer->drawPopupBox(Rect(X0, Y0, X1, Y1), panel_map);

  return Y0 + 1;
}

void Popup::showMessage(const string& message,
                        const bool DRAW_MAP_AND_INTERFACE,
                        const string title) const {
  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface(false);
  }

  vector<string> lines = eng->textFormatting->lineToLines(
                           message, TEXT_AREA_WIDTH);
  const int TEXT_AREA_HEIGHT =  int(lines.size()) + 3;

  const int TITLE_Y_POS = printBoxAndReturnTitleYPos(TEXT_AREA_HEIGHT);

  int yPos = TITLE_Y_POS;

  if(title != "") {
    eng->renderer->drawTextCentered(
      title, panel_map, Pos(MAP_X_CELLS_HALF, TITLE_Y_POS),
      clrNosferatuTealLgt, clrBlack, true);
  }

  const bool SHOW_MESSAGE_CENTERED = lines.size() == 1;

  for(unsigned int i = 0; i < lines.size(); i++) {
    yPos++;
    if(SHOW_MESSAGE_CENTERED) {
      eng->renderer->drawTextCentered(
        lines.at(i), panel_map, Pos(MAP_X_CELLS_HALF, yPos), clrWhite,
        clrBlack, true);
    } else {
      eng->renderer->drawText(
        lines.at(i), panel_map, Pos(TEXT_AREA_X0, yPos), clrWhite);
    }
    eng->log->addLineToHistory(lines.at(i));
  }
  yPos += 2;

  eng->renderer->drawTextCentered(
    "space/esc to close", panel_map, Pos(MAP_X_CELLS_HALF, yPos),
    clrNosferatuTeal);

  eng->renderer->updateScreen();

  eng->query->waitForEscOrSpace();

  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface();
  }
}

int Popup::showMultiChoiceMessage(const string& message,
                                  const bool DRAW_MAP_AND_INTERFACE,
                                  const vector<string>& choices,
                                  const string title) const {

  vector<string> lines = eng->textFormatting->lineToLines(
                           message, TEXT_AREA_WIDTH);
  const int TEXT_HEIGHT = int(lines.size());
  const int NR_CHOICES = int(choices.size());

  const int TEXT_AREA_HEIGHT = TEXT_HEIGHT + NR_CHOICES + 3;

  MenuBrowser browser(NR_CHOICES, 0);

  multiChoiceMessageDrawingHelper(
    lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
    TEXT_AREA_HEIGHT, title);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);

    switch(action) {
      case menuAction_browsed: {
        multiChoiceMessageDrawingHelper(
          lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
          TEXT_AREA_HEIGHT, title);
      } break;

      case menuAction_canceled: {
      } break;

      case menuAction_selected: {
        if(DRAW_MAP_AND_INTERFACE) {
          eng->renderer->drawMapAndInterface();
        }
        return browser.getPos().y;
      } break;

      case menuAction_selectedWithShift: {
      } break;
    }
  }
}

void Popup::multiChoiceMessageDrawingHelper(const vector<string>& lines,
    const vector<string>& choices, const bool DRAW_MAP_AND_INTERFACE,
    const unsigned int currentChoice, const int TEXT_AREA_HEIGHT,
    const string title) const {

  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface(false);
  }

  const int TITLE_Y_POS = printBoxAndReturnTitleYPos(TEXT_AREA_HEIGHT);

  int yPos = TITLE_Y_POS;

  if(title != "") {
    eng->renderer->drawTextCentered(
      title, panel_map, Pos(MAP_X_CELLS_HALF, TITLE_Y_POS),
      clrCyanLgt, clrBlack, true);
  }

  const bool SHOW_MESSAGE_CENTERED = lines.size() == 1;

  for(unsigned int i = 0; i < lines.size(); i++) {
    yPos++;
    if(SHOW_MESSAGE_CENTERED) {
      eng->renderer->drawTextCentered(
        lines.at(i), panel_map, Pos(MAP_X_CELLS_HALF, yPos),
        clrWhite, clrBlack, true);
    } else {
      eng->renderer->drawText(
        lines.at(i), panel_map, Pos(TEXT_AREA_X0, yPos), clrWhite);
    }
    eng->log->addLineToHistory(lines.at(i));
  }
  yPos++;

  for(unsigned int i = 0; i < choices.size(); i++) {
    yPos++;
    SDL_Color clr = i == currentChoice ? clrNosferatuTealLgt :
                    clrNosferatuTealDrk;
    eng->renderer->drawTextCentered(
      choices.at(i), panel_map, Pos(MAP_X_CELLS_HALF, yPos),
      clr, clrBlack, true);
  }
  eng->renderer->updateScreen();
}

