#include "Popup.h"

#include "Engine.h"
#include "Renderer.h"
#include "TextFormatting.h"
#include "Log.h"
#include "Query.h"
#include "CommonTypes.h"
#include "MenuBrowser.h"
#include "MenuInputHandler.h"
#include "Audio.h"

const int TEXT_W = 39;
const int TEXT_X0 = MAP_W_HALF - ((TEXT_W) / 2);

int Popup::printBoxAndReturnTitleYPos(const int TEXT_H) const {
  const int BOX_W = TEXT_W + 2;
  const int BOX_H = TEXT_H + 2;

  const int X0 = TEXT_X0 - 1;

  const int Y0 = MAP_H_HALF - (BOX_H / 2) - 1;
  const int X1 = X0 + BOX_W - 1;
  const int Y1 = Y0 + BOX_H - 1;

  eng.renderer->coverArea(panel_map, Pos(X0, Y0), Pos(BOX_W, BOX_H));
  eng.renderer->drawPopupBox(Rect(X0, Y0, X1, Y1), panel_map);

  return Y0 + 1;
}

void Popup::showMessage(const string& message,
                        const bool DRAW_MAP_AND_INTERFACE,
                        const string& title, const Sfx_t sfx) const {

  if(DRAW_MAP_AND_INTERFACE) {eng.renderer->drawMapAndInterface(false);}

  vector<string> lines;
  TextFormatting::lineToLines(message, TEXT_W, lines);
  const int TEXT_H =  int(lines.size()) + 3;

  int y = printBoxAndReturnTitleYPos(TEXT_H);

  if(sfx != endOfSfx) {eng.audio->play(sfx);}

  if(title.empty() == false) {
    eng.renderer->drawTextCentered(
      title, panel_map, Pos(MAP_W_HALF, y),
      /*clrNosfTealLgt*/ clrWhite, clrBlack, true);
  }

  const bool SHOW_MESSAGE_CENTERED = lines.size() == 1;

  for(string & line : lines) {
    y++;
    if(SHOW_MESSAGE_CENTERED) {
      eng.renderer->drawTextCentered(line, panel_map, Pos(MAP_W_HALF, y),
                                     clrWhite, clrBlack, true);
    } else {
      eng.renderer->drawText(line, panel_map, Pos(TEXT_X0, y), clrWhite);
    }
    eng.log->addLineToHistory(line);
  }
  y += 2;

  eng.renderer->drawTextCentered("space/esc to close", panel_map,
                                 Pos(MAP_W_HALF, y), clrNosfTeal);

  eng.renderer->updateScreen();

  eng.query->waitForEscOrSpace();

  if(DRAW_MAP_AND_INTERFACE) {eng.renderer->drawMapAndInterface();}
}

int Popup::showMultiChoiceMessage(const string& message,
                                  const bool DRAW_MAP_AND_INTERFACE,
                                  const vector<string>& choices,
                                  const string& title, const Sfx_t sfx) const {

  vector<string> lines;
  TextFormatting::lineToLines(message, TEXT_W, lines);
  const int NR_MSG_LINES  = int(lines.size());
  const int NR_CHOICES    = int(choices.size());

  const int TEXT_H = NR_MSG_LINES + NR_CHOICES + 3;

  MenuBrowser browser(NR_CHOICES, 0);

  if(sfx != endOfSfx) {eng.audio->play(sfx);}

  multiChoiceMessageDrawingHelper(
    lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
    TEXT_H, title);

  while(true) {
    const MenuAction_t action = eng.menuInputHandler->getAction(browser);

    switch(action) {
      case menuAction_browsed: {
        multiChoiceMessageDrawingHelper(
          lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
          TEXT_H, title);
      } break;

      case menuAction_esc:
      case menuAction_space: {
        if(DRAW_MAP_AND_INTERFACE) {
          eng.renderer->drawMapAndInterface();
        }
        return NR_CHOICES - 1;
      } break;

      case menuAction_selectedWithShift: {} break;

      case menuAction_selected: {
        if(DRAW_MAP_AND_INTERFACE) {
          eng.renderer->drawMapAndInterface();
        }
        return browser.getPos().y;
      } break;
    }
  }
}

void Popup::multiChoiceMessageDrawingHelper(
  const vector<string>& lines, const vector<string>& choices,
  const bool DRAW_MAP_AND_INTERFACE, const unsigned int currentChoice,
  const int TEXT_H, const string title) const {

  if(DRAW_MAP_AND_INTERFACE) {
    eng.renderer->drawMapAndInterface(false);
  }

  int y = printBoxAndReturnTitleYPos(TEXT_H);

  if(title.empty() == false) {
    eng.renderer->drawTextCentered(title, panel_map, Pos(MAP_W_HALF, y),
                                   clrCyanLgt, clrBlack, true);
  }

  const bool SHOW_MESSAGE_CENTERED = lines.size() == 1;

  for(const string & line : lines) {
    y++;
    if(SHOW_MESSAGE_CENTERED) {
      eng.renderer->drawTextCentered(line, panel_map, Pos(MAP_W_HALF, y),
                                     clrWhite, clrBlack, true);
    } else {
      eng.renderer->drawText(line, panel_map, Pos(TEXT_X0, y), clrWhite);
    }
    eng.log->addLineToHistory(line);
  }
  y++;

  for(unsigned int i = 0; i < choices.size(); i++) {
    y++;
    SDL_Color clr = i == currentChoice ? clrNosfTealLgt : clrNosfTealDrk;
    eng.renderer->drawTextCentered(
      choices.at(i), panel_map, Pos(MAP_W_HALF, y),
      clr, clrBlack, true);
  }
  eng.renderer->updateScreen();
}

