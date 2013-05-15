#include "Manual.h"

#include <fstream>
#include <iostream>

#include "Input.h"
#include "Engine.h"
#include "TextFormatting.h"
#include "Render.h"

void Manual::readFile() {
  string curLine;
  ifstream file("manual");

  vector<string> formated;

  if(file.is_open()) {
    while(getline(file, curLine)) {
      //The '$' symbol is used in the manual file for lines that should not be formated.
      if(curLine.size() > 0) {
        if(curLine.at(0) == '$') {
          curLine.erase(curLine.begin());
          lines.push_back(curLine);
        } else {
          formated.resize(0);
          formated = eng->textFormatting->lineToLines(curLine, MAP_X_CELLS - 2);

          for(unsigned int i = 0; i < formated.size(); i++) {
            lines.push_back(formated.at(i));
          }
        }
      } else {
        lines.push_back(curLine);
      }
    }
  }

  file.close();
}

void Manual::drawManualInterface() {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, 2);
  eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);

  eng->renderer->drawText(" Displaying manual ", renderArea_screen, 3, 1, clrWhite);

  eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);

  eng->renderer->drawText(" 2/8, down/up to navigate | space/esc to exit ", renderArea_characterLines, 3, 1, clrWhite);
}

void Manual::run() {
  eng->renderer->clearScreen();

  string str;

  int topElement = 0;
  int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

  drawManualInterface();

  int yCell = 2;
  for(int i = topElement; i <= btmElement; i++) {
    eng->renderer->drawText(lines.at(i), renderArea_screen, 1, yCell, clrWhite);
    yCell++;
  }

  eng->renderer->updateScreen();

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement = max(0, min(topElement + 3, static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawManualInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i), renderArea_screen, 1, yCell, clrWhite);
        yCell++;
      }
      eng->renderer->updateScreen();
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement = max(0, min(topElement - 3, static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawManualInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i), renderArea_screen, 1, yCell, clrWhite);
        yCell++;
      }
      eng->renderer->updateScreen();
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }
  }

  eng->renderer->coverRenderArea(renderArea_screen);
}
