#include "Credits.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Input.h"
#include "TextFormatting.h"
#include "Render.h"

using namespace std;

namespace Credits {

namespace {

vector<string> lines_;

void readFile() {
  lines_.clear();

  string curLine;
  ifstream file("credits.txt");

  vector<string> formattedLines;

  if(file.is_open()) {
    while(getline(file, curLine)) {
      if(curLine.empty()) {
        lines_.push_back(curLine);
      } else {
        TextFormatting::lineToLines(curLine, MAP_W - 2, formattedLines);
        for(string& line : formattedLines) {lines_.push_back(line);}
      }
    }
  }

  file.close();
}

} //namespace

void init() {
  readFile();
}

void run() {
  Render::clearScreen();

  string str;

  const string decorationLine(MAP_W, '-');

  const int X_LABEL = 3;

  Render::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  Render::drawText(" Displaying credits.txt ", Panel::screen,
                     Pos(X_LABEL, 0), clrGray);

  Render::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

  Render::drawText(" [space/esc] to exit ", Panel::screen,
                     Pos(X_LABEL, SCREEN_H - 1), clrGray);

  int yPos = 1;
  for(string& line : lines_) {
    Render::drawText(line, Panel::screen, Pos(0, yPos++), clrWhite);
  }

  Render::updateScreen();

  //Read keys
  while(true) {
    const KeyData& d = Input::readKeysUntilFound();
    if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE) {
      break;
    }
  }
}

} //Credits
