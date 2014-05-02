#include "Credits.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Input.h"
#include "TextFormatting.h"
#include "Renderer.h"

using namespace std;

namespace Credits {

namespace {

vector<string> lines_;

void readFile() {
  lines_.resize(0);

  string curLine;
  ifstream file("credits.txt");

  vector<string> formattedLines;

  if(file.is_open()) {
    while(getline(file, curLine)) {
      if(curLine.empty()) {
        lines_.push_back(curLine);
      } else {
        TextFormatting::lineToLines(curLine, MAP_W - 2, formattedLines);
        for(string & line : formattedLines) {lines_.push_back(line);}
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
  Renderer::clearScreen();

  string str;

  const string decorationLine(MAP_W, '-');

  const int X_LABEL = 3;

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  Renderer::drawText(" Displaying credits.txt ", Panel::screen,
                     Pos(X_LABEL, 0), clrGray);

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

  Renderer::drawText(" space/esc to exit ", Panel::screen,
                     Pos(X_LABEL, SCREEN_H - 1), clrGray);

  int yPos = 1;
  for(string & line : lines_) {
    Renderer::drawText(line, Panel::screen, Pos(0, yPos++), clrWhite);
  }

  Renderer::updateScreen();

  //Read keys
  while(true) {
    const KeyboardReadRetData& d = Input::readKeysUntilFound();
    if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      break;
    }
  }
}

} //Credits
