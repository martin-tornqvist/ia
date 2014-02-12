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

int Popup::printBoxAndGetTitleYPos(const int TEXT_H_TOT,
                                   const int TEXT_W_OVERRIDE) const {
  const int TEXT_W_USED = TEXT_W_OVERRIDE == - 1 ? TEXT_W : TEXT_W_OVERRIDE;
  const int BOX_W       = TEXT_W_USED + 2;
  const int BOX_H       = TEXT_H_TOT + 2;

  const int X0          = MAP_W_HALF - ((TEXT_W_USED) / 2) - 1;

  const int Y0          = MAP_H_HALF - (BOX_H / 2) - 1;
  const int X1          = X0 + BOX_W - 1;
  const int Y1          = Y0 + BOX_H - 1;

  eng.renderer->coverArea(panel_map, Pos(X0, Y0), Pos(BOX_W, BOX_H));
  eng.renderer->drawPopupBox(Rect(X0, Y0, X1, Y1), panel_map);

  return Y0 + 1;
}

void Popup::showMsg(const string& msg, const bool DRAW_MAP_AND_INTERFACE,
                    const string& title, const SfxId sfx) const {

  if(DRAW_MAP_AND_INTERFACE) {eng.renderer->drawMapAndInterface(false);}

  vector<string> lines;
  TextFormatting::lineToLines(msg, TEXT_W, lines);
  const int TEXT_H_TOT =  int(lines.size()) + 3;

  int y = printBoxAndGetTitleYPos(TEXT_H_TOT);

  if(sfx != endOfSfxId) {eng.audio->play(sfx);}

  if(title.empty() == false) {
    eng.renderer->drawTextCentered(
      title, panel_map, Pos(MAP_W_HALF, y),
      /*clrNosfTealLgt*/ clrWhite, clrBlack, true);
  }

  const bool SHOW_MSG_CENTERED = lines.size() == 1;

  for(string & line : lines) {
    y++;
    if(SHOW_MSG_CENTERED) {
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

int Popup::showMenuMsg(const string& msg, const bool DRAW_MAP_AND_INTERFACE,
                       const vector<string>& choices,
                       const string& title, const SfxId sfx) const {

  vector<string> lines;
  TextFormatting::lineToLines(msg, TEXT_W, lines);
  const int TITLE_H         = title.empty() ? 0 : 1;
  const int NR_MSG_LINES    = int(lines.size());
  const int NR_BLANK_LINES  = (NR_MSG_LINES == 0 && TITLE_H == 0) ? 0 : 1;
  const int NR_CHOICES      = int(choices.size());

  const int TEXT_H_TOT = TITLE_H + NR_MSG_LINES + NR_BLANK_LINES + NR_CHOICES;

  MenuBrowser browser(NR_CHOICES, 0);

  if(sfx != endOfSfxId) {eng.audio->play(sfx);}

  menuMsgDrawingHelper(
    lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
    TEXT_H_TOT, title);

  while(true) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);

    switch(action) {
      case menuAction_browsed: {
        menuMsgDrawingHelper(
          lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
          TEXT_H_TOT, title);
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

void Popup::menuMsgDrawingHelper(
  const vector<string>& lines, const vector<string>& choices,
  const bool DRAW_MAP_AND_INTERFACE, const unsigned int currentChoice,
  const int TEXT_H_TOT, const string title) const {

  if(DRAW_MAP_AND_INTERFACE) {
    eng.renderer->drawMapAndInterface(false);
  }

  //If no message lines, set width to widest menu option or title with
  int textWidthOverride = -1;
  if(lines.empty()) {
    textWidthOverride = title.size();
    for(const string & s : choices) {
      textWidthOverride = max(textWidthOverride, int(s.size()));
    }
    textWidthOverride += 2;
  }

  int y = printBoxAndGetTitleYPos(TEXT_H_TOT, textWidthOverride);

  if(title.empty() == false) {
    eng.renderer->drawTextCentered(title, panel_map, Pos(MAP_W_HALF, y),
                                   clrCyanLgt, clrBlack, true);
  }

  const bool SHOW_MSG_CENTERED = lines.size() == 1;

  for(const string & line : lines) {
    y++;
    if(SHOW_MSG_CENTERED) {
      eng.renderer->drawTextCentered(line, panel_map, Pos(MAP_W_HALF, y),
                                     clrWhite, clrBlack, true);
    } else {
      eng.renderer->drawText(line, panel_map, Pos(TEXT_X0, y), clrWhite);
    }
    eng.log->addLineToHistory(line);
  }
  if(lines.empty() == false || title.empty() == false) {y += 2;}

  for(size_t i = 0; i < choices.size(); i++) {
    SDL_Color clr = i == currentChoice ? clrNosfTealLgt : clrNosfTealDrk;
    eng.renderer->drawTextCentered(
      choices.at(i), panel_map, Pos(MAP_W_HALF, y),
      clr, clrBlack, true);
    y++;
  }
  eng.renderer->updateScreen();
}

