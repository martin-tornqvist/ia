#include "Manual.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "Input.h"
#include "TextFormatting.h"
#include "Renderer.h"

using namespace std;

namespace Manual {

namespace {

vector<string> lines_;

void readFile() {
  string curLine;
  ifstream file("manual.txt");

  vector<string> formatted;

  if(file.is_open()) {
    while(getline(file, curLine)) {
      if(curLine.empty()) {
        lines_.push_back(curLine);
      } else {
        //Do not format lines that start with two spaces
        bool shouldFormatLine = true;
        if(curLine.size() > 1) {
          if(curLine.at(0) == ' ' && curLine.at(1) == ' ') {
            shouldFormatLine = false;
          }
        }
        if(shouldFormatLine) {
          TextFormatting::lineToLines(curLine, MAP_W - 3, formatted);
          for(unsigned int i = 0; i < formatted.size(); i++) {
            lines_.push_back(formatted.at(i));
          }
        } else {
          curLine.erase(curLine.begin());
          lines_.push_back(curLine);
        }
      }
    }
  }

  file.close();
}

void drawManualInterface() {
  const string decorationLine(MAP_W, '-');

  const int X_LABEL = 3;

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  Renderer::drawText(" Displaying manual ", Panel::screen, Pos(X_LABEL, 0),
                     clrGray);

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

  Renderer::drawText(" 2/8, down/up, j/k to navigate | space/esc to exit ",
                     Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);
}

} //namespace

void init() {
  readFile();
}

void run() {
  const int LINE_JUMP           = 3;
  const int NR_LINES_TOT        = lines_.size();
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

  int topNr = 0;
  int btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

  while(true) {
    Renderer::clearScreen();
    drawManualInterface();
    int yPos = 1;
    for(int i = topNr; i <= btmNr; i++) {
      Renderer::drawText(lines_.at(i), Panel::screen, Pos(0, yPos++),
                         clrWhite);
    }
    Renderer::updateScreen();

    const KeyboardReadRetData& d = Input::readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN || d.key_ == 'j') {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR) {
        topNr = 0;
      } else {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP || d.key_ == 'k') {
      topNr = max(0, topNr - LINE_JUMP);
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      break;
    }
    btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
  }
}

} //Manual
