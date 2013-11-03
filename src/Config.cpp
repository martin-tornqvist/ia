#include "Config.h"

#include <fstream>
#include <iostream>

#include "Converters.h"
#include "Engine.h"
#include "MenuInputHandler.h"
#include "MenuBrowser.h"
#include "Query.h"
#include "Render.h"
#include "Input.h"

using namespace std;

Config::Config(Engine* engine) :
  GAME_VERSION("v15.0 alpha"),
  TILES_IMAGE_NAME("images/gfx_16x24.png"),
  MAIN_MENU_LOGO_IMAGE_NAME("images/main_menu_logo.png"),
  fontBig(""),
  LOG_X_CELLS_OFFSET(1), LOG_Y_CELLS_OFFSET(1),
  LOG_X_CELLS(MAP_X_CELLS - LOG_X_CELLS_OFFSET),
  CHARACTER_LINES_Y_CELLS_OFFSET(LOG_Y_CELLS_OFFSET + 1 + MAP_Y_CELLS),
  CHARACTER_LINES_Y_CELLS(3),
  SCREEN_BPP(32),
  FRAMES_PER_SECOND(30),
  PLAYER_START_X(10),
  PLAYER_START_Y(MAP_Y_CELLS_HALF),
  isBotPlaying(false),
  MAINSCREEN_Y_CELLS_OFFSET(LOG_Y_CELLS_OFFSET + 1),
  eng(engine) {

  fontImageNames.resize(0);
  fontImageNames.push_back("images/8x12.png");
  fontImageNames.push_back("images/11x19.png");
  fontImageNames.push_back("images/16x24_clean_v1.png");
  fontImageNames.push_back("images/16x24_clean_v2.png");
  fontImageNames.push_back("images/16x24_typewriter.png");

  setDefaultVariables();

  vector<string> lines;
  readFile(lines);
  if(lines.empty()) {
    collectLinesFromVariables(lines);
    writeLinesToFile(lines);
  } else {
    setAllVariablesFromLines(lines);
  }
  setCellDimDependentVariables();
}

void Config::runOptionsMenu() {
  MenuBrowser browser(14, 0);
  vector<string> lines;

  const int OPTION_VALUES_X_POS = 40;
  const int OPTIONS_Y_POS = 3;

  draw(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        draw(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
      } break;

      case menuAction_canceled: {
        //Since ASCII mode wall symbol may have changed,
        //we need to redefine the feature data list
        eng->featureDataHandler->initDataList();
        return;
      } break;

      case menuAction_selected: {
        draw(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
        playerSetsOption(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
        collectLinesFromVariables(lines);
        writeLinesToFile(lines);
        draw(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
      } break;

      default: {} break;
    }
  }
}

void Config::playerSetsOption(const MenuBrowser* const browser,
                              const int OPTION_VALUES_X_POS,
                              const int OPTIONS_Y_POS) {
  switch(browser->getPos().y) {
    case 0: {
      isTilesMode = !isTilesMode;
      if(isTilesMode) {
        if(cellW == 8 && cellH == 12) {
          fontScale = 2;
        } else {
          if(cellW != 16 || cellH != 24) {
            fontScale = 1;
            fontBig = "images/16x24_clean_v1.png";
          }
        }
      }
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      eng->renderer->initAndClearPrev();
    }
    break;

    case 1: {
      if(isTilesMode) {
        fontScale = 1;
      }

      for(unsigned int i = 0; i < fontImageNames.size(); i++) {
        if(fontBig == fontImageNames.at(i)) {
          fontBig = i == fontImageNames.size() - 1 ?
                    fontImageNames.front() :
                    fontImageNames.at(i + 1);
          break;
        }
      }
      parseFontNameAndSetCellDims();

      if(isTilesMode) {
        if(cellW == 8 && cellH == 12) {
          fontScale = 2;
          parseFontNameAndSetCellDims();
        }

        while(cellW != 16 || cellH != 24) {
          for(unsigned int i = 0; i < fontImageNames.size(); i++) {
            if(fontBig == fontImageNames.at(i)) {
              fontBig = i == fontImageNames.size() - 1 ?
                        fontImageNames.front() :
                        fontImageNames.at(i + 1);
              break;
            }
          }
          parseFontNameAndSetCellDims();
        }
      }

      setCellDimDependentVariables();
      eng->renderer->initAndClearPrev();
    }
    break;

    case 2: {
      if(fontScale == 1) {
        if(isTilesMode == false) {
          fontScale = 2;
        }
      } else {
        if(isTilesMode == false) {
          fontScale = 1;
        }
      }
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      eng->renderer->initAndClearPrev();
    } break;

    case 3: {
      isFullscreen = !isFullscreen;
      eng->renderer->initAndClearPrev();
    } break;

    case 4: {
      isAsciiWallSymbolFullSquare = !isAsciiWallSymbolFullSquare;
    } break;

    case 5: {isIntroLevelSkipped = !isIntroLevelSkipped;} break;

    case 6: {useRangedWpnMeleeePrompt = !useRangedWpnMeleeePrompt;} break;

    case 7: {useRangedWpnAutoReload = !useRangedWpnAutoReload;} break;

    case 8: {
      const int NR = eng->query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosferatuSepiaLgt, 1, 3, keyRepeatDelay, true);
      if(NR != -1) {
        keyRepeatDelay = NR;
        eng->input->setKeyRepeatDelays();
      }
    } break;

    case 9: {
      const int NR = eng->query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosferatuSepiaLgt, 1, 3, keyRepeatInterval, true);
      if(NR != -1) {
        keyRepeatInterval = NR;
        eng->input->setKeyRepeatDelays();
      }
    } break;

    case 10: {
      const int NR = eng->query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosferatuSepiaLgt, 1, 3, delayProjectileDraw, true);
      if(NR != -1) {
        delayProjectileDraw = NR;
      }
    } break;

    case 11: {
      const int NR = eng->query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosferatuSepiaLgt, 1, 3, delayShotgun, true);
      if(NR != -1) {
        delayShotgun = NR;
      }
    } break;

    case 12: {
      const int NR = eng->query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosferatuSepiaLgt, 1, 3, delayExplosion, true);
      if(NR != -1) {
        delayExplosion = NR;
      }
    } break;

    case 13: {
      setDefaultVariables();
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      eng->renderer->initAndClearPrev();
    } break;
  }
}

void Config::draw(const MenuBrowser* const browser,
                  const int OPTION_VALUES_X_POS,
                  const int OPTIONS_Y_POS) {

  const SDL_Color clrActive     = clrNosferatuSepiaLgt;
  const SDL_Color clrInactive   = clrNosferatuSepiaDrk;

  eng->renderer->clearScreen();

  int optionNr = 0;

  const int X0 = 1;
  const int X1 = OPTION_VALUES_X_POS;
  const int Y0 = OPTIONS_Y_POS;

  string str = "";

  eng->renderer->drawText("-Options-", panel_screen, Pos(X0, Y0 - 1), clrWhite);

  eng->renderer->drawText("Use tile set", panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  str = isTilesMode ? "Yes" : "No";
  eng->renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  eng->renderer->drawText("Font", panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(fontBig, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  eng->renderer->drawText("Scale font 2x", panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  str = fontScale == 2 ? "Yes" : "No";
  eng->renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  eng->renderer->drawText("Fullscreen (experimental)", panel_screen,
                          Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(isFullscreen ? "Yes" : "No",
                          panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Ascii mode wall symbol";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ? clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  str = isAsciiWallSymbolFullSquare ? "Full square" : "Hash sign";
  eng->renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  eng->renderer->drawText("Skip intro level", panel_screen,
                          Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  str = isIntroLevelSkipped ? "Yes" : "No";
  eng->renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Ranged weapon melee attack warning";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  str = useRangedWpnMeleeePrompt ? "Yes" : "No";
  eng->renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Ranged weapon auto reload";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  str = useRangedWpnAutoReload ? "Yes" : "No";
  eng->renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Key repeat delay (ms)";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(toString(keyRepeatDelay), panel_screen,
                          Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Key repeat interval (ms)";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(toString(keyRepeatInterval), panel_screen,
                          Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Projectile delay (ms)";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(toString(delayProjectileDraw), panel_screen,
                          Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Shotgun delay (ms)";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(toString(delayShotgun), panel_screen,
                          Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Explosion delay (ms)";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  eng->renderer->drawText(toString(delayExplosion), panel_screen,
                          Pos(X1, Y0 + optionNr),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);
  optionNr++;

  str = "Reset to defaults";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr + 1),
                          browser->getPos().y == optionNr ?
                          clrActive : clrInactive);

  str = "space/esc to confirm changes";
  eng->renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr + 4),
                          clrWhite);

  eng->renderer->updateScreen();
}

void Config::setCellDimDependentVariables() {
  mainscreenHeight        = MAP_Y_CELLS * cellH;
  logOffsetX              = LOG_X_CELLS_OFFSET * cellW;
  logOffsetY              = LOG_Y_CELLS_OFFSET * cellH;
  logHeight               = cellH;
  mainscreenOffsetY       = MAINSCREEN_Y_CELLS_OFFSET * cellH;
  characterLinesOffsetY   = logOffsetY + logHeight + mainscreenHeight;
  CHARACTER_LINES_HEIGHT  = CHARACTER_LINES_Y_CELLS * cellH;
  screenWidth             = MAP_X_CELLS * cellW;
  screenHeight            = characterLinesOffsetY + CHARACTER_LINES_HEIGHT;
}

void Config::parseFontNameAndSetCellDims() {
  trace << "Config::parseFontNameAndSetCellDims()..." << endl;
  string fontName = fontBig;

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

  trace << "Config: Parsed font image name, found dims: ";
  trace << widthStr << "x" << heightStr << endl;

  cellW = toInt(widthStr)  * fontScale;
  cellH = toInt(heightStr) * fontScale;
  trace << "Config::parseFontNameAndSetCellDims() [DONE]" << endl;
}

void Config::setDefaultVariables() {
  trace << "Config::setDefaultVariables()..." << endl;
  isTilesMode = true;
  fontBig = "images/16x24_clean_v2.png";
  fontScale = 1;
  parseFontNameAndSetCellDims();
  isFullscreen = false;
  isAsciiWallSymbolFullSquare = true;
  isIntroLevelSkipped = false;
  useRangedWpnMeleeePrompt = true;
  useRangedWpnAutoReload = false;
  keyRepeatDelay = 130;
  keyRepeatInterval = 60;
  delayProjectileDraw = 45;
  delayShotgun = 120;
  delayExplosion = 350;
  trace << "Config::setDefaultVariables() [DONE]" << endl;
}

void Config::collectLinesFromVariables(vector<string>& lines) {
  trace << "Config::collectLinesFromVariables()..." << endl;
  lines.resize(0);
  lines.push_back(isTilesMode == false ? "0" : "1");
  lines.push_back(toString(fontScale));
  lines.push_back(fontBig);
  lines.push_back(isFullscreen == false ? "0" : "1");
  lines.push_back(isAsciiWallSymbolFullSquare == false ? "0" : "1");
  lines.push_back(isIntroLevelSkipped == false ? "0" : "1");
  lines.push_back(useRangedWpnMeleeePrompt == false ? "0" : "1");
  lines.push_back(useRangedWpnAutoReload == false ? "0" : "1");
  lines.push_back(toString(keyRepeatDelay));
  lines.push_back(toString(keyRepeatInterval));
  lines.push_back(toString(delayProjectileDraw));
  lines.push_back(toString(delayShotgun));
  lines.push_back(toString(delayExplosion));
  trace << "Config::collectLinesFromVariables() [DONE]" << endl;
}

void Config::toggleFullscreen() {
  SDL_Surface* screenCpy = SDL_DisplayFormat(eng->renderer->screenSurface_);

  isFullscreen = !isFullscreen;
  parseFontNameAndSetCellDims();
  setCellDimDependentVariables();
  eng->renderer->initAndClearPrev();

  eng->renderer->applySurface(Pos(0, 0), screenCpy, NULL);
  eng->renderer->updateScreen();

  vector<string> lines;
  collectLinesFromVariables(lines);
  writeLinesToFile(lines);
}

void Config::setAllVariablesFromLines(vector<string>& lines) {
  trace << "Config::setAllVariablesFromLines()..." << endl;
  string curLine = "";

  curLine = lines.front();
  if(curLine == "0") {
    isTilesMode = false;
  } else {
    isTilesMode = true;
    if(cellW != 16 || cellH != 24) {
      fontBig = "images/16x24_clean_v1.png";
      parseFontNameAndSetCellDims();
    }
  }
  lines.erase(lines.begin());

  curLine = lines.front();
  fontScale = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  fontBig = curLine;
  parseFontNameAndSetCellDims();
  lines.erase(lines.begin());

  curLine = lines.front();
  isFullscreen = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isAsciiWallSymbolFullSquare = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isIntroLevelSkipped = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  useRangedWpnMeleeePrompt = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  useRangedWpnAutoReload = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  keyRepeatDelay = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  keyRepeatInterval = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  delayProjectileDraw = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  delayShotgun = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  delayExplosion = toInt(curLine);
  lines.erase(lines.begin());
  trace << "Config::setAllVariablesFromLines() [DONE]" << endl;
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


