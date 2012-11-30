#include "Log.h"

#include <algorithm>

#include "Input.h"
#include "Engine.h"
#include "Converters.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"

using namespace std;

void MessageLog::clearLog() {
  if(line.empty() == false) {

    history.push_back(line);

    while(history.size() > 300) {
      history.erase(history.begin());
    }

    line.resize(0);
    drawLog();
  }
}

void MessageLog::drawLine(const vector<Message>& lineToDraw, const int yCell) const {
  sf::Color clr;
  string str;
  int drawXpos;

  const unsigned int LINE_SIZE = lineToDraw.size();

  for(unsigned int i = 0; i < LINE_SIZE; i++) {
    const Message& curMessage = lineToDraw.at(i);
    clr = curMessage.clr;
    str = curMessage.str;
    if(curMessage.repeats > 1) {
      str += curMessage.strRepeats;
    }

    drawXpos = findCurXpos(lineToDraw, i);

    eng->renderer->drawText(str, renderArea_log, drawXpos, yCell, clr);
  }
}

void MessageLog::drawLog() const {
  drawLine(line, 0);
}

void MessageLog::displayHistory() {
  clearLog();

  eng->renderer->clearWindow();

  string str;

  int topElement = max(0, static_cast<int>(history.size()) - static_cast<int>(MAP_Y_CELLS));
  int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(history.size()) - 1);
  drawHistoryInterface(topElement, btmElement);
  int yCell = 1;
  for(int i = topElement; i <= btmElement; i++) {
    drawLine(history.at(static_cast<unsigned int>(i)), yCell);
    yCell++;
  }

  eng->renderer->updateWindow();

  const int LINE_JUMP = 3;

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sfmlKey_ == sf::Keyboard::Down) {
      topElement = max(0, min(topElement + LINE_JUMP, static_cast<int>(history.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(history.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawHistoryInterface(topElement, btmElement);
      yCell = 1;
      for(int i = topElement; i <= btmElement; i++) {
        drawLine(history.at(static_cast<unsigned int>(i)), yCell);
        yCell++;
      }
      eng->renderer->updateWindow();
    }
    else if(d.key_ == '8' || d.sfmlKey_ == sf::Keyboard::Up) {
      topElement = max(0, min(topElement - LINE_JUMP, static_cast<int>(history.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(history.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawHistoryInterface(topElement, btmElement);
      yCell = 1;
      for(int i = topElement; i <= btmElement; i++) {
        drawLine(history.at(static_cast<unsigned int>(i)), yCell);
        yCell++;
      }
      eng->renderer->updateWindow();
    }
    else if(d.sfmlKey_ == sf::Keyboard::Space || d.sfmlKey_ == sf::Keyboard::Escape) {
      done = true;
    }
  }

//  eng->renderer->clearWindow();
  eng->renderer->drawMapAndInterface();
}

void MessageLog::drawHistoryInterface(const int topLine, const int bottomLine) const {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverRenderArea(renderArea_log);
  eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);
  if(history.empty()) {
    eng->renderer->drawText(" No message history ", renderArea_screen, 3, 1, clrWhite);
  } else {
    eng ->renderer->drawText(" Displaying messages " + intToString(topLine) + "-" + intToString(bottomLine) + " of "
                             + intToString(history.size()) + " ", renderArea_screen, 3, 1, clrWhite);
  }

  eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);
  eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);
}

int MessageLog::findCurXpos(const vector<Message>& afterLine, const unsigned int messageNr) const {
  if(messageNr == 0) {
    return 0;
  }

  const unsigned int LINE_SIZE = afterLine.size();

  if(LINE_SIZE == 0) {
    return 0;
  }

  int xPos = 0;

  for(unsigned int i = 0; i < messageNr; i++) {

    const Message& curMessage = afterLine.at(i);

    xPos += static_cast<int>(curMessage.str.length());

    if(curMessage.repeats > 1) {
      xPos += 4;
    }

    xPos++;
  }

  return xPos;
}

void MessageLog::addMessage(const string& text, const sf::Color color, MessageInterrupt_t interrupt) {
  bool repeated = false;

  //New message equal to previous?
  if(line.empty() == false) {
    if(line.back().str.compare(text) == 0) {
      line.back().addRepeat();
      repeated = true;
    }
  }

  if(repeated == false) {
    const int REPEAT_LABEL_LENGTH = 4;
    const int MORE_PROMPT_LENGTH = 7;

    const int CUR_X_POS = findCurXpos(line, line.size());

    const bool MESSAGE_FITS = CUR_X_POS + static_cast<int>(text.size()) + REPEAT_LABEL_LENGTH + MORE_PROMPT_LENGTH < MAP_X_CELLS;

    if(MESSAGE_FITS == false) {
      eng->renderer->drawMapAndInterface(false);
      eng->renderer->drawText("[MORE]", renderArea_log, CUR_X_POS, 0, clrCyanLight);
      eng->renderer->updateWindow();
      eng->query->waitForKeyPress();
      clearLog();
      eng->renderer->drawMapAndInterface(true);
    }

    const Message m(text, color);
    line.push_back(m);
  }

  drawLog();

  //Messages may stop long actions like first aid and auto travel.
  if(interrupt != messageInterrupt_never) {
    eng->player->interruptActions(interrupt == messageInterrupt_query);
  }
}

