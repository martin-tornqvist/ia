#include "Log.h"

#include <algorithm>

#include "Input.h"
#include "Engine.h"
#include "Converters.h"
#include "Renderer.h"
#include "Query.h"
#include "ActorPlayer.h"

using namespace std;

void Log::clearLog() {
  if(line.empty() == false) {
    history.push_back(line);
    while(history.size() > 300) {history.erase(history.begin());}
    line.resize(0);
  }
}

void Log::drawLine(const vector<Message>& lineToDraw, const int yCell) const {
  string str = "";

  const unsigned int LINE_SIZE = lineToDraw.size();
  for(unsigned int i = 0; i < LINE_SIZE; i++) {
    const Message& curMsg = lineToDraw.at(i);
    str = curMsg.str;
    if(curMsg.repeats > 1) {
      str += curMsg.strRepeats;
    }

    const int DRAW_X_POS = findCurXpos(lineToDraw, i);
    eng.renderer->drawText(str, panel_log, Pos(DRAW_X_POS, yCell), curMsg.clr);
  }
}

void Log::drawLog(const bool SHOULD_UPDATE_SCREEN) const {
  eng.renderer->coverArea(panel_log, Pos(0, 0), Pos(MAP_W, 1));
  drawLine(line, 0);
  if(SHOULD_UPDATE_SCREEN) eng.renderer->updateScreen();
}

void Log::addMsg(const string& text, const SDL_Color color,
                 const bool INTERRUPT_PLAYER_ACTIONS,
                 const bool FORCE_MORE_PROMPT) {

  eng.renderer->drawMapAndInterface(true);

  bool repeated = false;

  //New message equal to previous?
  if(line.empty() == false) {
    if(line.back().str.compare(text) == 0) {
      line.back().addRepeat();
      repeated = true;
    }
  }

  if(repeated == false) {
    const int REPEAT_LEN  = 4;
    const int MORE_LEN    = 9;

    const int CUR_X_POS = findCurXpos(line, line.size());

    const bool IS_MSG_FIT =
      CUR_X_POS + int(text.size()) + REPEAT_LEN + MORE_LEN < MAP_W;

    if(IS_MSG_FIT == false) {
      drawLog(false);
      eng.renderer->drawText(
        "--More--", panel_log, Pos(CUR_X_POS, 0), clrBlack, clrGray);
      eng.renderer->updateScreen();
      eng.query->waitForKeyPress();
      clearLog();
    }

    const Message m(text, color);
    line.push_back(m);
  }

  if(FORCE_MORE_PROMPT) {
    drawLog(false);
    const int CUR_X_POS_AFTER = findCurXpos(line, line.size());
    eng.renderer->drawText("--More--", panel_log,
                           Pos(CUR_X_POS_AFTER, 0), clrBlack, clrGray);
    eng.renderer->updateScreen();
    eng.query->waitForKeyPress();
    clearLog();
  }

  drawLog(true);

  //Messages may stop long actions like first aid and auto travel.
  if(INTERRUPT_PLAYER_ACTIONS) {
    eng.player->interruptActions();
  }
}

void Log::displayHistory() {
  clearLog();

  eng.renderer->clearScreen();

  int topElement = max(0, int(history.size()) - int(MAP_H));
  int btmElement = min(topElement + MAP_H - 1, int(history.size()) - 1);
  drawHistoryInterface(topElement, btmElement);
  int yCell = 1;
  for(int i = topElement; i <= btmElement; i++) {
    drawLine(history.at((unsigned int)i), yCell);
    yCell++;
  }

  eng.renderer->updateScreen();

  const int LINE_JUMP = 3;

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng.input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement = min(topElement + LINE_JUMP,
                       int(history.size()) - int(MAP_H));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_H - 1,
                       int(history.size()) - 1);
      eng.renderer->coverArea(panel_screen, Pos(0, 2),
                              Pos(MAP_W, MAP_H));
      drawHistoryInterface(topElement, btmElement);
      yCell = 1;
      for(int i = topElement; i <= btmElement; i++) {
        drawLine(history.at((unsigned int)i), yCell);
        yCell++;
      }
      eng.renderer->updateScreen();
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement = min(topElement - LINE_JUMP,
                       int(history.size()) - int(MAP_H));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_H - 1, int(history.size()) - 1);
      eng.renderer->coverArea(panel_screen, Pos(0, 2),
                              Pos(MAP_W, MAP_H));
      drawHistoryInterface(topElement, btmElement);
      yCell = 1;
      for(int i = topElement; i <= btmElement; i++) {
        drawLine(history.at((unsigned int)i), yCell);
        yCell++;
      }
      eng.renderer->updateScreen();
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }
  }

  eng.renderer->drawMapAndInterface();
}

void Log::drawHistoryInterface(const int topLine, const int bottomLine) const {
  const string decorationLine(MAP_W - 2, '-');

  eng.renderer->coverPanel(panel_log);
  eng.renderer->drawText(decorationLine, panel_screen, Pos(0, 0), clrWhite);
  if(history.empty()) {
    eng.renderer->drawText(" No message history ", panel_screen,
                           Pos(3, 0), clrWhite);
  } else {
    eng.renderer->drawText(
      " Displaying messages " + toString(topLine) + "-" +
      toString(bottomLine) + " of " +
      toString(history.size()) + " ", panel_screen, Pos(3, 0), clrWhite);
  }

  eng.renderer->drawText(
    decorationLine, panel_char, Pos(0, 2), clrWhite);
  eng.renderer->drawText(
    " 2/8, down/up to navigate | space/esc to exit ",
    panel_char, Pos(3, 2), clrWhite);
}

int Log::findCurXpos(const vector<Message>& afterLine,
                     const unsigned int messageNr) const {
  if(messageNr == 0) {return 0;}

  const unsigned int LINE_SIZE = afterLine.size();

  if(LINE_SIZE == 0) {return 0;}

  int xPos = 0;

  for(unsigned int i = 0; i < messageNr; i++) {
    const Message& curMessage = afterLine.at(i);
    xPos += int(curMessage.str.length());
    if(curMessage.repeats > 1) {xPos += 4;}
    xPos++;
  }

  return xPos;
}

