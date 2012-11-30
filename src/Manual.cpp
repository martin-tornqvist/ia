#include "Manual.h"

#include <fstream>
#include <iostream>

#include "Input.h"
#include "Engine.h"
#include "TextFormatting.h"
#include "Render.h"

void Manual::readFile() {
  string curLine;
  ifstream file("manual.txt");

  vector<string> formated;

  if(file.is_open()) {
    while(getline(file, curLine)) {
      //The '$' symbol is used in manual.txt for lines that should not be formated.
      if(curLine.size() > 0) {
        if(curLine.at(0) == '$') {
          curLine.erase(curLine.begin());
          lines.push_back(curLine);
        } else {
          formated.resize(0);
          formated = eng->textFormatting->lineToLines(curLine, MAP_X_CELLS);

          for(unsigned int i = 0; i < formated.size(); i++) {
            lines.push_back(formated.at(i));
          }
        }
      } else {
        lines.push_back(curLine);
      }
    }
  } else {
  }

  file.close();
}

void Manual::drawManualInterface() {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, 2);
  eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);

  eng->renderer->drawText("Displaying manual.txt", renderArea_screen, 3, 1, clrWhite);

  eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);

  eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);
}

void Manual::run() {
  eng->renderer->clearWindow();

  string str;

  int topElement = 0;
  int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

  drawManualInterface();

  int yCell = 2;
  for(int i = topElement; i <= btmElement; i++) {
    eng->renderer->drawText(lines.at(i), renderArea_screen, 1, yCell, clrRedLight);
    yCell++;
  }

  eng->renderer->updateWindow();

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sfmlKey_ == sf::Keyboard::Down) {
      topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawManualInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i), renderArea_screen, 1, yCell, clrRedLight);
        yCell++;
      }
      eng->renderer->updateWindow();
    }
    else if(d.key_ == '8' || d.sfmlKey_ == sf::Keyboard::Up) {
      topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawManualInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i), renderArea_screen, 1, yCell, clrRedLight);
        yCell++;
      }
      eng->renderer->updateWindow();
    }
    else if(d.sfmlKey_ == sf::Keyboard::Space || d.sfmlKey_ == sf::Keyboard::Escape) {
      done = true;
    }
  }

  eng->renderer->coverRenderArea(renderArea_screen);
}
