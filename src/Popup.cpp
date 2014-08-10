#include "Popup.h"

#include "Config.h"
#include "Renderer.h"
#include "TextFormatting.h"
#include "Log.h"
#include "Query.h"
#include "CmnTypes.h"
#include "MenuBrowser.h"
#include "MenuInputHandling.h"
#include "Audio.h"

using namespace std;

namespace Popup {

namespace {

const int TEXT_W  = 39;
const int TEXT_X0 = MAP_W_HALF - ((TEXT_W) / 2);

int printBoxAndGetTitleYPos(const int TEXT_H_TOT,
                            const int TEXT_W_OVERRIDE = -1) {

  const int TEXT_W_USED = TEXT_W_OVERRIDE == - 1 ? TEXT_W : TEXT_W_OVERRIDE;
  const int BOX_W       = TEXT_W_USED + 2;
  const int BOX_H       = TEXT_H_TOT + 2;

  const int X0          = MAP_W_HALF - ((TEXT_W_USED) / 2) - 1;

  const int Y0          = MAP_H_HALF - (BOX_H / 2) - 1;
  const int X1          = X0 + BOX_W - 1;
  const int Y1          = Y0 + BOX_H - 1;

  Renderer::coverArea(Panel::map, Pos(X0, Y0), Pos(BOX_W, BOX_H));
  Renderer::drawPopupBox(Rect(X0, Y0, X1, Y1), Panel::map);

  return Y0 + 1;
}

void menuMsgDrawingHelper(
  const vector<string>& lines, const vector<string>& choices,
  const bool DRAW_MAP_AND_INTERFACE, const unsigned int curChoice,
  const int TEXT_H_TOT, const string& title) {

  if(DRAW_MAP_AND_INTERFACE) {
    Renderer::drawMapAndInterface(false);
  }

  //If no message lines, set width to widest menu option or title with
  int textWidthOverride = -1;
  if(lines.empty()) {
    textWidthOverride = title.size();
    for(const string& s : choices) {
      textWidthOverride = max(textWidthOverride, int(s.size()));
    }
    textWidthOverride += 2;
  }

  int y = printBoxAndGetTitleYPos(TEXT_H_TOT, textWidthOverride);

  if(!title.empty()) {
    Renderer::drawTextCentered(title, Panel::map, Pos(MAP_W_HALF, y),
                               clrCyanLgt, clrBlack, true);
  }

  const bool SHOW_MSG_CENTERED = lines.size() == 1;

  for(const string& line : lines) {
    y++;
    if(SHOW_MSG_CENTERED) {
      Renderer::drawTextCentered(line, Panel::map, Pos(MAP_W_HALF, y),
                                 clrWhite, clrBlack, true);
    } else {
      Renderer::drawText(line, Panel::map, Pos(TEXT_X0, y), clrWhite);
    }
    Log::addLineToHistory(line);
  }
  if(!lines.empty() || !title.empty()) {y += 2;}

  for(size_t i = 0; i < choices.size(); ++i) {
    Clr clr = i == curChoice ? clrNosfTealLgt : clrNosfTealDrk;
    Renderer::drawTextCentered(
      choices.at(i), Panel::map, Pos(MAP_W_HALF, y),
      clr, clrBlack, true);
    y++;
  }
  Renderer::updateScreen();
}

} //namespace

void showMsg(const string& msg, const bool DRAW_MAP_AND_INTERFACE,
             const string& title, const SfxId sfx) {

  if(DRAW_MAP_AND_INTERFACE) {Renderer::drawMapAndInterface(false);}

  vector<string> lines;
  TextFormatting::lineToLines(msg, TEXT_W, lines);
  const int TEXT_H_TOT =  int(lines.size()) + 3;

  int y = printBoxAndGetTitleYPos(TEXT_H_TOT);

  if(sfx != SfxId::END) {Audio::play(sfx);}

  if(!title.empty()) {
    Renderer::drawTextCentered(
      title, Panel::map, Pos(MAP_W_HALF, y),
      /*clrNosfTealLgt*/ clrWhite, clrBlack, true);
  }

  const bool SHOW_MSG_CENTERED = lines.size() == 1;

  for(string& line : lines) {
    y++;
    if(SHOW_MSG_CENTERED) {
      Renderer::drawTextCentered(line, Panel::map, Pos(MAP_W_HALF, y),
                                 clrWhite, clrBlack, true);
    } else {
      Renderer::drawText(line, Panel::map, Pos(TEXT_X0, y), clrWhite);
    }
    Log::addLineToHistory(line);
  }
  y += 2;

  Renderer::drawTextCentered("[space/esc] to close", Panel::map,
                             Pos(MAP_W_HALF, y), clrNosfTeal);

  Renderer::updateScreen();

  Query::waitForEscOrSpace();

  if(DRAW_MAP_AND_INTERFACE) {Renderer::drawMapAndInterface();}
}

int showMenuMsg(const string& msg, const bool DRAW_MAP_AND_INTERFACE,
                const vector<string>& choices,
                const string& title, const SfxId sfx) {

  if(Config::isBotPlaying()) {return 0;}

  vector<string> lines;
  TextFormatting::lineToLines(msg, TEXT_W, lines);
  const int TITLE_H         = title.empty() ? 0 : 1;
  const int NR_MSG_LINES    = int(lines.size());
  const int NR_BLANK_LINES  = (NR_MSG_LINES == 0 && TITLE_H == 0) ? 0 : 1;
  const int NR_CHOICES      = int(choices.size());

  const int TEXT_H_TOT = TITLE_H + NR_MSG_LINES + NR_BLANK_LINES + NR_CHOICES;

  MenuBrowser browser(NR_CHOICES, 0);

  if(sfx != SfxId::END) {Audio::play(sfx);}

  menuMsgDrawingHelper(
    lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
    TEXT_H_TOT, title);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);

    switch(action) {
      case MenuAction::browsed: {
        menuMsgDrawingHelper(
          lines, choices, DRAW_MAP_AND_INTERFACE, browser.getPos().y,
          TEXT_H_TOT, title);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        if(DRAW_MAP_AND_INTERFACE) {
          Renderer::drawMapAndInterface();
        }
        return NR_CHOICES - 1;
      } break;

      case MenuAction::selectedShift: {} break;

      case MenuAction::selected: {
        if(DRAW_MAP_AND_INTERFACE) {
          Renderer::drawMapAndInterface();
        }
        return browser.getPos().y;
      } break;
    }
  }
}

} //Popup
