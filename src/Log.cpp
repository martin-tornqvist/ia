#include "Log.h"

#include <algorithm>

#include "Input.h"
#include "Engine.h"
#include "Converters.h"
#include "Renderer.h"
#include "Query.h"
#include "ActorPlayer.h"

using namespace std;

const string moreStr = "--More--";

void Log::clearLog() {
  for(vector<Msg>& line : lines) {
    if(line.empty() == false) {
      history.push_back(line);
      while(history.size() > 300) {history.erase(history.begin());}
      line.resize(0);
    }
  }
}

void Log::drawLine(const vector<Msg>& lineToDraw, const int Y_POS) const {
  for(const Msg & msg : lineToDraw) {
    string str = "";
    msg.getStrWithRepeats(str);
    Renderer::drawText(str, Panel::log, Pos(msg.xPos_, Y_POS), msg.clr_);
  }
}

void Log::drawLog(const bool SHOULD_UPDATE_SCREEN) const {
  Renderer::coverArea(Panel::log, Pos(0, 0), Pos(MAP_W, 2));
  for(int i = 0; i < 2; i++) drawLine(lines[i], i);
  if(SHOULD_UPDATE_SCREEN) Renderer::updateScreen();
}

void Log::promptAndClearLog() {
  drawLog(false);

  int xPos    = 0;
  int lineNr = lines[1].empty() ? 0 : 1;

  if(lines[lineNr].empty() == false) {
    Msg* const lastMsg = &lines[lineNr].back();
    xPos = getXAfterMsg(lastMsg);
    if(lineNr == 0) {
      if(xPos + int(moreStr.size()) - 1 >= MAP_W) {
        xPos    = 0;
        lineNr  = 1;
      }
    }
  }

  Renderer::drawText(
    moreStr, Panel::log, Pos(xPos, lineNr), clrBlack, clrGray);

  Renderer::updateScreen();
  eng.query->waitForKeyPress();
  clearLog();
}

void Log::addMsg(const string& text, const SDL_Color& clr,
                 const bool INTERRUPT_PLAYER_ACTIONS,
                 const bool ADD_MORE_PROMPT_AFTER_MSG) {

  assert(text.empty() == false);
  assert(text.at(0) != ' ');

  int curLineNr = lines[1].empty() ? 0 : 1;

  Msg* lastMsg = NULL;
  if(lines[curLineNr].empty() == false) {lastMsg = &lines[curLineNr].back();}

  bool isRepeated = false;

  //Check if message is identical to previous
  if(ADD_MORE_PROMPT_AFTER_MSG == false && lastMsg != NULL) {
    string str = "";
    lastMsg->getStrRaw(str);
    if(str.compare(text) == 0) {
      lastMsg->incrRepeat();
      isRepeated = true;
    }
  }

  if(isRepeated == false) {
    const int REPEAT_STR_LEN = 4;
    const int PADDING_LEN = REPEAT_STR_LEN +
                            (curLineNr == 0 ? 0 : (moreStr.size() + 1));
    int xPos              = getXAfterMsg(lastMsg);

    const bool IS_MSG_FIT = xPos + int(text.size()) + PADDING_LEN - 1 < MAP_W;

    if(IS_MSG_FIT == false) {
      if(curLineNr == 0) {
        curLineNr = 1;
      } else {
        promptAndClearLog();
        curLineNr = 0;
      }
      xPos = 0;
    }

    lines[curLineNr].push_back(Msg(text, clr, xPos));
  }

  if(ADD_MORE_PROMPT_AFTER_MSG) {promptAndClearLog();}

  //Messages may stop long actions like first aid and auto travel.
  if(INTERRUPT_PLAYER_ACTIONS) {
    eng.player->interruptActions();
  }
}

void Log::displayHistory() {
  clearLog();

  const int LINE_JUMP           = 3;
  const int NR_LINES_TOT        = history.size();
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

  int topNr = max(0, NR_LINES_TOT - MAX_NR_LINES_ON_SCR);
  int btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

  while(true) {
    Renderer::clearScreen();
    drawHistoryInterface(topNr, btmNr);
    int yPos = 1;
    for(int i = topNr; i <= btmNr; i++) {
      drawLine(history.at(i), yPos++);
    }
    Renderer::updateScreen();

    const KeyboardReadRetData& d = Input::readKeysUntilFound(eng);
    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR) {
        topNr = 0;
      } else {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topNr = max(0, topNr - LINE_JUMP);
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      break;
    }
    btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
  }

  Renderer::drawMapAndInterface();
}

void Log::drawHistoryInterface(const int TOP_LINE_NR,
                               const int BTM_LINE_NR) const {
  const string decorationLine(MAP_W, '-');

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  const int X_LABEL = 3;

  if(history.empty()) {
    Renderer::drawText(" No message history ", Panel::screen,
                       Pos(X_LABEL, 0), clrGray);
  } else {
    Renderer::drawText(
      " Displaying messages " + toString(TOP_LINE_NR + 1) + "-" +
      toString(BTM_LINE_NR + 1) + " of " +
      toString(history.size()) + " ", Panel::screen, Pos(X_LABEL, 0), clrGray);
  }

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

  Renderer::drawText(" 2/8, down/up to navigate | space/esc to exit ",
                     Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);
}

