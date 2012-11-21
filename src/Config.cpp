#include "Config.h"

#include <fstream>
#include <iostream>

#include "Converters.h"
#include "Engine.h"
#include "MenuInputHandler.h"
#include "Query.h"
#include "Render.h"

using namespace std;

Config::Config(Engine* engine) :
  GAME_VERSION("v14.0"),
  TILES_IMAGE_NAME("images/gfx_16x24.bmp"),
  MAIN_MENU_LOGO_IMAGE_NAME("images/main_menu_logo.bmp"),
  LOG_X_CELLS_OFFSET(1), LOG_Y_CELLS_OFFSET(1),
  LOG_X_CELLS(MAP_X_CELLS - LOG_X_CELLS_OFFSET),
  CHARACTER_LINES_Y_CELLS_OFFSET(LOG_Y_CELLS_OFFSET + 1 + MAP_Y_CELLS),
  CHARACTER_LINES_Y_CELLS(3),
  SCREEN_BPP(32),
  FRAMES_PER_SECOND(30),
  PLAYER_START_X(10),
  PLAYER_START_Y(MAP_Y_CELLS_HALF),
  BOT_PLAYING(false),
  MAINSCREEN_Y_CELLS_OFFSET(LOG_Y_CELLS_OFFSET + 1),
  eng(engine) {

  fontImageNames.resize(0);
  fontImageNames.push_back("images/8x12.bmp");
  fontImageNames.push_back("images/11x19.bmp");
  fontImageNames.push_back("images/16x24_clean_v1.bmp");
  fontImageNames.push_back("images/16x24_clean_v2.bmp");
  fontImageNames.push_back("images/16x24_typewriter.bmp");

  vector<string> lines;
  readFile(lines);
  if(lines.empty()) {
    setDefaultVariables();
    collectLinesFromVariables(lines);
    writeLinesToFile(lines);
  } else {
    setAllVariablesFromLines(lines);
  }
  setCellDimDependentVariables();
}

void Config::setCellDimDependentVariables() {
  MAINSCREEN_WIDTH = MAP_X_CELLS * CELL_W;
  MAINSCREEN_HEIGHT = MAP_Y_CELLS * CELL_H;

  LOG_X_OFFSET = LOG_X_CELLS_OFFSET * CELL_W;
  LOG_Y_OFFSET = LOG_Y_CELLS_OFFSET * CELL_H;

  LOG_WIDTH = LOG_X_CELLS * CELL_W;
  LOG_HEIGHT = CELL_H;

  MAINSCREEN_Y_OFFSET = MAINSCREEN_Y_CELLS_OFFSET * CELL_H;
  CHARACTER_LINES_Y_OFFSET = LOG_Y_OFFSET + LOG_HEIGHT + MAINSCREEN_HEIGHT;

  CHARACTER_LINES_HEIGHT = CHARACTER_LINES_Y_CELLS * CELL_H;

  SCREEN_WIDTH = MAP_X_CELLS * CELL_W;
  SCREEN_HEIGHT = CHARACTER_LINES_Y_OFFSET + CHARACTER_LINES_HEIGHT;
}

void Config::runOptionsMenu() {
  MenuBrowser browser(12, 0);
  vector<string> lines;

  const int OPTION_VALUES_X_POS = 40;
  const int OPTIONS_Y_POS = 3;

  draw(browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      draw(browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
    }
    break;

    case menuAction_canceled: {
      // Since ASCII mode wall symbol may have changed, we need to redefine the feature data list
      eng->featureData->makeList();
      return;
    }
    break;

    case menuAction_selected: {
      playerSetsOption(browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
      collectLinesFromVariables(lines);
      writeLinesToFile(lines);
      draw(browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
    }
    }
  }
}

void Config::parseFontNameAndSetCellDims() {
  tracer << "Config:: parseFontNameAndSetCellDims()..." << endl;
  string fontName = FONT_IMAGE_NAME;

  char ch = 'a';
  while(ch < '0' || ch > '9') {
    fontName.erase(fontName.begin());
    ch = fontName.at(0);
  }

  string widthStr = "";
  while(ch != 'x') {
    fontName.erase(fontName.begin());
    widthStr += ch;
    ch = fontName.at(0);
  }

  fontName.erase(fontName.begin());
  ch = fontName.at(0);

  string heightStr = "";
  while(ch != '_' && ch != '.') {
    fontName.erase(fontName.begin());
    heightStr += ch;
    ch = fontName.at(0);
  }

  tracer << "Config: Parsed font image name, found dims: " << widthStr << "x" << heightStr << endl;

  CELL_W = stringToInt(widthStr);
  CELL_H = stringToInt(heightStr);
  tracer << "Config:: parseFontNameAndSetCellDims() [DONE]" << endl;
}

void Config::setDefaultVariables() {
  USE_TILE_SET = true;
  FONT_IMAGE_NAME = fontImageNames.back();
  parseFontNameAndSetCellDims();
  FULLSCREEN = false;
  WALL_SYMBOL_FULL_SQUARE = false;
  SKIP_INTRO_LEVEL = false;
  RANGED_WPN_MELEE_PROMPT = true;
  KEY_REPEAT_DELAY = 180;
  KEY_REPEAT_INTERVAL = 50;
  DELAY_PROJECTILE_DRAW = 13;
  DELAY_SHOTGUN = 85;
  DELAY_EXPLOSION = 180;
}

void Config::collectLinesFromVariables(vector<string>& lines) {
  lines.resize(0);
  lines.push_back(USE_TILE_SET == false ? "0" : "1");
  lines.push_back(FONT_IMAGE_NAME);
  lines.push_back(FULLSCREEN == false ? "0" : "1");
  lines.push_back(WALL_SYMBOL_FULL_SQUARE == false ? "0" : "1");
  lines.push_back(SKIP_INTRO_LEVEL == false ? "0" : "1");
  lines.push_back(RANGED_WPN_MELEE_PROMPT == false ? "0" : "1");
  lines.push_back(intToString(KEY_REPEAT_DELAY));
  lines.push_back(intToString(KEY_REPEAT_INTERVAL));
  lines.push_back(intToString(DELAY_PROJECTILE_DRAW));
  lines.push_back(intToString(DELAY_SHOTGUN));
  lines.push_back(intToString(DELAY_EXPLOSION));
}

void Config::draw(const MenuBrowser& browser, const int OPTION_VALUES_X_POS,
                  const int OPTIONS_Y_POS) {

  const SDL_Color clrSelected = clrWhite;
  const SDL_Color clrGeneral = clrRedLight;

  eng->renderer->clearRenderArea(renderArea_screen);

  int optionNr = 0;

  const int X0 = 1;
  const int X1 = OPTION_VALUES_X_POS;
  const int Y0 = OPTIONS_Y_POS;

  eng->renderer->drawText("-OPTIONS-", renderArea_screen, X0, Y0 - 1, clrWhite);

  eng->renderer->drawText("TILE MODE (16x24 FONT REQUIRED)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(USE_TILE_SET ? "YES" : "NO", renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("FONT", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(FONT_IMAGE_NAME, renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("FULLSCREEN (EXPERIMENTAL)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(FULLSCREEN ? "YES" : "NO", renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("ASCII MODE WALL SYMBOL", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(WALL_SYMBOL_FULL_SQUARE ? "FULL SQUARE" : "HASH SIGN", renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("SKIP INTRO LEVEL", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(SKIP_INTRO_LEVEL ? "YES" : "NO", renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("RANGED WEAPON MELEE WARNING", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(RANGED_WPN_MELEE_PROMPT ? "YES" : "NO", renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("KEY REPEAT DELAY (ms)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(intToString(KEY_REPEAT_DELAY), renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("KEY REPEAT INTERVAL (ms)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(intToString(KEY_REPEAT_INTERVAL), renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("PROJECTILE DELAY (ms)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(intToString(DELAY_PROJECTILE_DRAW), renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("SHOTGUN DELAY (ms)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(intToString(DELAY_SHOTGUN), renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("EXPLOSION DELAY (ms)", renderArea_screen, X0, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(":", renderArea_screen, X1 - 2, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  eng->renderer->drawText(intToString(DELAY_EXPLOSION), renderArea_screen, X1, Y0 + optionNr, browser.getPos().y == optionNr ? clrSelected : clrGeneral);
  optionNr++;

  eng->renderer->drawText("RESET TO DEFAULTS", renderArea_screen, X0, Y0 + optionNr + 1, browser.getPos().y == optionNr ? clrSelected : clrGeneral);

  eng->renderer->drawText("[space/esc] done", renderArea_screen, X0, Y0 + optionNr + 4, clrWhite);

  eng->renderer->flip();
}

void Config::playerSetsOption(const MenuBrowser& browser, const int OPTION_VALUES_X_POS, const int OPTIONS_Y_POS) {
  switch(browser.getPos().y) {
  case 0: {
    USE_TILE_SET = !USE_TILE_SET;
    if(USE_TILE_SET) {
      if(CELL_W != 16 || CELL_H != 24) {
        FONT_IMAGE_NAME = "images/16x24_clean_v1.bmp";
      }
    }
    parseFontNameAndSetCellDims();
    setCellDimDependentVariables();
    eng->renderer->setGfxAnVideoMode();
  } break;

  case 1: {
    for(unsigned int i = 0; i < fontImageNames.size(); i++) {
      if(FONT_IMAGE_NAME == fontImageNames.at(i)) {
        FONT_IMAGE_NAME = i == fontImageNames.size() - 1 ? fontImageNames.front() : fontImageNames.at(i + 1);
        break;
      }
    }
    parseFontNameAndSetCellDims();

    if(USE_TILE_SET) {
      while(CELL_W != 16 && CELL_H != 24) {
        for(unsigned int i = 0; i < fontImageNames.size(); i++) {
          if(FONT_IMAGE_NAME == fontImageNames.at(i)) {
            FONT_IMAGE_NAME = i == fontImageNames.size() - 1 ? fontImageNames.front() : fontImageNames.at(i + 1);
            break;
          }
        }
        parseFontNameAndSetCellDims();
      }
    }

    setCellDimDependentVariables();
    eng->renderer->setGfxAnVideoMode();
  } break;

  case 2: {
    FULLSCREEN = !FULLSCREEN;
    eng->renderer->setGfxAnVideoMode();
  } break;

  case 3: {
    WALL_SYMBOL_FULL_SQUARE = !WALL_SYMBOL_FULL_SQUARE;
  } break;

  case 4: {
    SKIP_INTRO_LEVEL = !SKIP_INTRO_LEVEL;
  } break;

  case 5: {
    RANGED_WPN_MELEE_PROMPT = !RANGED_WPN_MELEE_PROMPT;
  } break;

  case 6: {
    const int NR = eng->query->number(
                     coord(OPTION_VALUES_X_POS , OPTIONS_Y_POS + browser.getPos().y),
                     clrWhite, 1, 3, KEY_REPEAT_DELAY);
    if(NR != -1) {
      KEY_REPEAT_DELAY = NR;
      eng->renderer->setKeyDelays();
    }
  } break;

  case 7: {
    const int NR = eng->query->number(
                     coord(OPTION_VALUES_X_POS , OPTIONS_Y_POS + browser.getPos().y),
                     clrWhite, 1, 3, KEY_REPEAT_INTERVAL);
    if(NR != -1) {
      KEY_REPEAT_INTERVAL = NR;
      eng->renderer->setKeyDelays();
    }
  } break;

  case 8: {
    const int NR = eng->query->number(
                     coord(OPTION_VALUES_X_POS , OPTIONS_Y_POS + browser.getPos().y),
                     clrWhite, 1, 3, DELAY_PROJECTILE_DRAW);
    if(NR != -1) {
      DELAY_PROJECTILE_DRAW = NR;
    }
  } break;

  case 9: {
    const int NR = eng->query->number(
                     coord(OPTION_VALUES_X_POS , OPTIONS_Y_POS + browser.getPos().y),
                     clrWhite, 1, 3, DELAY_SHOTGUN);
    if(NR != -1) {
      DELAY_SHOTGUN = NR;
    }
  } break;

  case 10: {
    const int NR = eng->query->number(
                     coord(OPTION_VALUES_X_POS , OPTIONS_Y_POS + browser.getPos().y),
                     clrWhite, 1, 3, DELAY_EXPLOSION);
    if(NR != -1) {
      DELAY_EXPLOSION = NR;
    }
  } break;

  case 11: {
    setDefaultVariables();
    parseFontNameAndSetCellDims();
    setCellDimDependentVariables();
    eng->renderer->setGfxAnVideoMode();
  } break;
  }
}

void Config::setAllVariablesFromLines(vector<string>& lines) {
  string curLine = "";

  curLine = lines.front();
  if(curLine == "0") {
    USE_TILE_SET = false;
  } else {
    USE_TILE_SET = true;
    if(CELL_W != 16 || CELL_H != 24) {
      FONT_IMAGE_NAME = "images/16x24_clean_v1.bmp";
      parseFontNameAndSetCellDims();
    }
  }
  lines.erase(lines.begin());

  curLine = lines.front();
  FONT_IMAGE_NAME = curLine;
  parseFontNameAndSetCellDims();
  lines.erase(lines.begin());

  curLine = lines.front();
  FULLSCREEN = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  WALL_SYMBOL_FULL_SQUARE = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  SKIP_INTRO_LEVEL = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  RANGED_WPN_MELEE_PROMPT = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  KEY_REPEAT_DELAY = stringToInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  KEY_REPEAT_INTERVAL = stringToInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  DELAY_PROJECTILE_DRAW = stringToInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  DELAY_SHOTGUN = stringToInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  DELAY_EXPLOSION = stringToInt(curLine);
  lines.erase(lines.begin());
}

void Config::writeLinesToFile(vector<string>& lines) {
  ofstream file;
  file.open("config", ios::trunc);

  for(unsigned int i = 0; i < lines.size(); i++) {
    file << lines.at(i);
    if(i != lines.size() - 1) {
      file << endl;
    }
  }

  file.close();
}

void Config::readFile(vector<string>& lines) {
  ifstream file;
  file.open("config");
  if(file.is_open()) {
    string line;
    while(getline(file, line)) {
      lines.push_back(line);
    }
    file.close();
  }
}


