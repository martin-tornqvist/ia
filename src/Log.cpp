#include "Log.h"

#include <algorithm>
#include <vector>
#include <string>

#include "Init.h"
#include "Input.h"
#include "Converters.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "Map.h"

using namespace std;

namespace Log {

namespace {

vector<Msg>           lines_[2];
vector< vector<Msg> > history_;
const string          moreStr = "--More--";

int getXAfterMsg(const Msg* const msg) {
  if(!msg) {return 0;}

  string str = "";
  msg->getStrWithRepeats(str);
  return msg->xPos_ + str.size() + 1;
}

void promptAndClearLog() {
  drawLog(false);

  int xPos    = 0;
  int lineNr = lines_[1].empty() ? 0 : 1;

  if(!lines_[lineNr].empty()) {
    Msg* const lastMsg = &lines_[lineNr].back();
    xPos = getXAfterMsg(lastMsg);
    if(lineNr == 0) {
      if(xPos + int(moreStr.size()) - 1 >= MAP_W) {
        xPos    = 0;
        lineNr  = 1;
      }
    }
  }

  Render::drawText(moreStr, Panel::log, Pos(xPos, lineNr), clrBlack, clrGray);

  Render::updateScreen();
  Query::waitForEscOrSpace();
  clearLog();
}

void drawHistoryInterface(const int TOP_LINE_NR, const int BTM_LINE_NR) {
  const string decorationLine(MAP_W, '-');

  Render::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  const int X_LABEL = 3;

  if(history_.empty()) {
    Render::drawText(" No message history ", Panel::screen,
                     Pos(X_LABEL, 0), clrGray);
  } else {
    Render::drawText(
      " Displaying messages " + toStr(TOP_LINE_NR + 1) + "-" +
      toStr(BTM_LINE_NR + 1) + " of " +
      toStr(history_.size()) + " ", Panel::screen, Pos(X_LABEL, 0), clrGray);
  }

  Render::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1), clrGray);

  Render::drawText(infoScrCmdInfo, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);
}

//Used by normal log and history viewer
void drawLine(const vector<Msg>& lineToDraw, const int Y_POS) {
  for(const Msg& msg : lineToDraw) {
    string str = "";
    msg.getStrWithRepeats(str);
    Render::drawText(str, Panel::log, Pos(msg.xPos_, Y_POS), msg.clr_);
  }
}

} //namespace

void init() {
  clearLog();
}

void clearLog() {
  for(vector<Msg>& line : lines_) {
    if(!line.empty()) {
      history_.push_back(line);
      while(history_.size() > 300) {history_.erase(history_.begin());}
      line.clear();
    }
  }
}

void drawLog(const bool SHOULD_UPDATE_SCREEN) {
  Render::coverArea(Panel::log, Pos(0, 0), Pos(MAP_W, 2));
  for(int i = 0; i < 2; ++i) drawLine(lines_[i], i);
  if(SHOULD_UPDATE_SCREEN) Render::updateScreen();
}

void addMsg(const string& text, const Clr& clr, const bool INTERRUPT_PLAYER_ACTIONS,
            const bool ADD_MORE_PROMPT_AFTER_MSG) {

  assert(!text.empty());
#ifndef NDEBUG
  if(text.at(0) == ' ') {
    TRACE << "Message starts with space: \"" << text << "\"" << endl;
    assert(false);
  }
#endif

  int curLineNr = lines_[1].empty() ? 0 : 1;

  Msg* lastMsg = nullptr;
  if(!lines_[curLineNr].empty()) {lastMsg = &lines_[curLineNr].back();}

  bool isRepeated = false;

  //Check if message is identical to previous
  if(!ADD_MORE_PROMPT_AFTER_MSG && lastMsg) {
    string str = "";
    lastMsg->getStrRaw(str);
    if(str.compare(text) == 0) {
      lastMsg->incrRepeat();
      isRepeated = true;
    }
  }

  if(!isRepeated) {
    const int REPEAT_STR_LEN = 4;

    const int PADDING_LEN = REPEAT_STR_LEN + (curLineNr == 0 ? 0 : (moreStr.size() + 1));

    int xPos = getXAfterMsg(lastMsg);

    const bool IS_MSG_FIT = xPos + int(text.size()) + PADDING_LEN - 1 < MAP_W;

    if(!IS_MSG_FIT) {
      if(curLineNr == 0) {
        curLineNr = 1;
      } else {
        promptAndClearLog();
        curLineNr = 0;
      }
      xPos = 0;
    }

    lines_[curLineNr].push_back(Msg(text, clr, xPos));
  }

  if(ADD_MORE_PROMPT_AFTER_MSG) {promptAndClearLog();}

  //Messages may stop long actions like first aid and quick walk
  if(INTERRUPT_PLAYER_ACTIONS) {
    Map::player->interruptActions();
  }
}

void displayHistory() {
  clearLog();

  const int LINE_JUMP           = 3;
  const int NR_LINES_TOT        = history_.size();
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

  int topNr = max(0, NR_LINES_TOT - MAX_NR_LINES_ON_SCR);
  int btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

  while(true) {
    Render::clearScreen();
    drawHistoryInterface(topNr, btmNr);
    int yPos = 1;
    for(int i = topNr; i <= btmNr; ++i) {
      drawLine(history_.at(i), yPos++);
    }
    Render::updateScreen();

    const KeyData& d = Input::readKeysUntilFound();
    if(d.key == '2' || d.sdlKey == SDLK_DOWN || d.key == 'j') {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR) {
        topNr = 0;
      } else {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    } else if(d.key == '8' || d.sdlKey == SDLK_UP || d.key == 'k') {
      topNr = max(0, topNr - LINE_JUMP);
    } else if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE) {
      break;
    }
    btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
  }

  Render::drawMapAndInterface();
}

void addLineToHistory(const string& lineToAdd) {
  vector<Msg> historyLine;
  historyLine.push_back(Msg(lineToAdd, clrWhite, 0));
  history_.push_back(historyLine);
}

const vector< vector<Msg> >& getHistory() {
  return history_;
}

} //Log
