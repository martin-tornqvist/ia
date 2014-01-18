#include "Config.h"

#include <fstream>
#include <iostream>

#include <SDL_image.h>

#include "Engine.h"

#include "Converters.h"
#include "MenuInputHandler.h"
#include "MenuBrowser.h"
#include "Query.h"
#include "Renderer.h"
#include "Input.h"
#include "Audio.h"
#include "TextFormatting.h"

using namespace std;

Config::Config(Engine& engine) :
  fontBig(""),
  isBotPlaying(false),
  eng(engine) {

  fontImageNames.resize(0);
  fontImageNames.push_back("images/8x12_DOS.png");
  fontImageNames.push_back("images/11x19.png");
  fontImageNames.push_back("images/11x22.png");
  fontImageNames.push_back("images/12x24.png");
  fontImageNames.push_back("images/16x24_v1.png");
  fontImageNames.push_back("images/16x24_v2.png");
  fontImageNames.push_back("images/16x24_DOS.png");
  fontImageNames.push_back("images/16x24_typewriter.png");

  setDefaultVariables();

  vector<string> lines;
  readFile(lines);
  if(lines.empty()) {
    collectLinesFromVariables(lines);
  } else {
    setAllVariablesFromLines(lines);
  }
  setCellDimDependentVariables();
}

void Config::runOptionsMenu() {
  MenuBrowser browser(16, 0);
  vector<string> lines;

  const int OPTION_VALUES_X_POS = 40;
  const int OPTIONS_Y_POS = 3;

  draw(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);

  while(true) {
    const MenuAction_t action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        draw(&browser, OPTION_VALUES_X_POS, OPTIONS_Y_POS);
      } break;

      case menuAction_esc:
      case menuAction_space: {
        //Since ASCII mode wall symbol may have changed,
        //we need to redefine the feature data list
        eng.featureDataHandler->initDataList();
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
      isAudioEnabled = !isAudioEnabled;
      eng.audio->initAndClearPrev();
    } break;

    case 1: {
      isTilesMode = !isTilesMode;
      if(isTilesMode && (cellW != 16 || cellH != 24)) {
        fontBig = "images/16x24_v1.png";
      }
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      eng.renderer->initAndClearPrev();
    } break;

    case 2: {
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
      eng.renderer->initAndClearPrev();
    } break;

    case 3: {
      isFullscreen = !isFullscreen;
      eng.renderer->initAndClearPrev();
    } break;

    case 4: {
      isTilesWallSymbolFullSquare = !isTilesWallSymbolFullSquare;
    } break;

    case 5: {
      isAsciiWallSymbolFullSquare = !isAsciiWallSymbolFullSquare;
    } break;

    case 6: {isIntroLevelSkipped = !isIntroLevelSkipped;} break;

    case 7: {useRangedWpnMeleeePrompt = !useRangedWpnMeleeePrompt;} break;

    case 8: {useRangedWpnAutoReload = !useRangedWpnAutoReload;} break;

    case 9: {
      const int NR = eng.query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosfTealLgt, 1, 3, keyRepeatDelay, true);
      if(NR != -1) {
        keyRepeatDelay = NR;
        eng.input->setKeyRepeatDelays();
      }
    } break;

    case 10: {
      const int NR = eng.query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosfTealLgt, 1, 3, keyRepeatInterval, true);
      if(NR != -1) {
        keyRepeatInterval = NR;
        eng.input->setKeyRepeatDelays();
      }
    } break;

    case 11: {
      const int NR = eng.query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosfTealLgt, 1, 3, delayProjectileDraw, true);
      if(NR != -1) {
        delayProjectileDraw = NR;
      }
    } break;

    case 12: {
      const int NR = eng.query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosfTealLgt, 1, 3, delayShotgun, true);
      if(NR != -1) {
        delayShotgun = NR;
      }
    } break;

    case 13: {
      const int NR = eng.query->number(
                       Pos(OPTION_VALUES_X_POS,
                           OPTIONS_Y_POS + browser->getPos().y),
                       clrNosfTealLgt, 1, 3, delayExplosion, true);
      if(NR != -1) {
        delayExplosion = NR;
      }
    } break;

    case 14: {
      setDefaultVariables();
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      eng.renderer->initAndClearPrev();
    } break;
  }
}

void Config::draw(const MenuBrowser* const browser,
                  const int OPTION_VALUES_X_POS,
                  const int OPTIONS_Y_POS) {

  const SDL_Color clrActive     = clrNosfTealLgt;
  const SDL_Color clrInactive   = clrNosfTealDrk;

  eng.renderer->clearScreen();

  int optionNr = 0;

  const int X0 = 1;
  const int X1 = OPTION_VALUES_X_POS;
  const int Y0 = OPTIONS_Y_POS;

  string str = "";

  eng.renderer->drawText("-Options-", panel_screen, Pos(X0, Y0 - 1), clrWhite);

  eng.renderer->drawText("Play audio", panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = isAudioEnabled ? "Yes" : "No";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  eng.renderer->drawText("Use tile set", panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = isTilesMode ? "Yes" : "No";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  eng.renderer->drawText("Font", panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  string fontDispName;
  TextFormatting::replaceAll(fontBig,       "images/",  "",   fontDispName);
  TextFormatting::replaceAll(fontDispName,  "_",        " ",  fontDispName);
  TextFormatting::replaceAll(fontDispName,  ".png",     "",   fontDispName);
  eng.renderer->drawText(fontDispName, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  eng.renderer->drawText("Fullscreen (experimental)", panel_screen,
                         Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(isFullscreen ? "Yes" : "No",
                         panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Tiles mode wall symbol";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ? clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = isTilesWallSymbolFullSquare ? "Full square" : "Pseudo-3D";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Ascii mode wall symbol";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ? clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = isAsciiWallSymbolFullSquare ? "Full square" : "Hash sign";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  eng.renderer->drawText("Skip intro level", panel_screen,
                         Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = isIntroLevelSkipped ? "Yes" : "No";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Ranged weapon melee attack warning";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = useRangedWpnMeleeePrompt ? "Yes" : "No";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Ranged weapon auto reload";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  str = useRangedWpnAutoReload ? "Yes" : "No";
  eng.renderer->drawText(str, panel_screen, Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Key repeat delay (ms)";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(toString(keyRepeatDelay), panel_screen,
                         Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Key repeat interval (ms)";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(toString(keyRepeatInterval), panel_screen,
                         Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Projectile delay (ms)";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(toString(delayProjectileDraw), panel_screen,
                         Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Shotgun delay (ms)";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(toString(delayShotgun), panel_screen,
                         Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Explosion delay (ms)";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(":", panel_screen, Pos(X1 - 2, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  eng.renderer->drawText(toString(delayExplosion), panel_screen,
                         Pos(X1, Y0 + optionNr),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);
  optionNr++;

  str = "Reset to defaults";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr + 1),
                         browser->getPos().y == optionNr ?
                         clrActive : clrInactive);

  str = "space/esc to confirm changes";
  eng.renderer->drawText(str, panel_screen, Pos(X0, Y0 + optionNr + 4),
                         clrWhite);

  eng.renderer->updateScreen();
}

void Config::setCellDimDependentVariables() {
  mapPixelH             = cellH * MAP_H;
  mapPixelOffsetH       = cellH * MAP_OFFSET_H;
  logPixelH             = cellH * LOG_H;
  charLinesPixelH       = cellH * CHAR_LINES_H;
  charLinesPixelOffsetH = cellH * CHAR_LINES_OFFSET_H;
  screenPixelW          = cellW * SCREEN_W;
  screenPixelH          = cellH * SCREEN_H;
}

void Config::parseFontNameAndSetCellDims() {
  trace << "Config::parseFontNameAndSetCellDims()..." << endl;
  string fontName = fontBig;

  char ch = 'a';
  while(ch < '0' || ch > '9') {
    fontName.erase(fontName.begin());
    ch = fontName.at(0);
  }

  string wStr = "";
  while(ch != 'x') {
    fontName.erase(fontName.begin());
    wStr += ch;
    ch = fontName.at(0);
  }

  fontName.erase(fontName.begin());
  ch = fontName.at(0);

  string hStr = "";
  while(ch != '_' && ch != '.') {
    fontName.erase(fontName.begin());
    hStr += ch;
    ch = fontName.at(0);
  }

  trace << "Config: Parsed font image name, found dims: ";
  trace << wStr << "x" << hStr << endl;

  cellW = toInt(wStr);
  cellH = toInt(hStr);
  trace << "Config::parseFontNameAndSetCellDims() [DONE]" << endl;
}

void Config::setDefaultVariables() {
  trace << "Config::setDefaultVariables()..." << endl;
  isAudioEnabled = true;
  isTilesMode = true;
  fontBig = "images/16x24_v2.png";
  parseFontNameAndSetCellDims();
  isFullscreen = false;
  isTilesWallSymbolFullSquare = false;
  isAsciiWallSymbolFullSquare = false;
  isIntroLevelSkipped = false;
  useRangedWpnMeleeePrompt = true;
  useRangedWpnAutoReload = false;
  keyRepeatDelay = 130;
  keyRepeatInterval = 60;
  delayProjectileDraw = 50;
  delayShotgun = 120;
  delayExplosion = 350;
  trace << "Config::setDefaultVariables() [DONE]" << endl;
}

void Config::collectLinesFromVariables(vector<string>& lines) {
  trace << "Config::collectLinesFromVariables()..." << endl;
  lines.resize(0);
  lines.push_back(isAudioEnabled == false ? "0" : "1");
  lines.push_back(isTilesMode == false ? "0" : "1");
  lines.push_back(fontBig);
  lines.push_back(isFullscreen == false ? "0" : "1");
  lines.push_back(isTilesWallSymbolFullSquare == false ? "0" : "1");
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
  SDL_Surface* screenCpy = SDL_DisplayFormat(eng.renderer->screenSurface_);

  isFullscreen = !isFullscreen;
  parseFontNameAndSetCellDims();
  setCellDimDependentVariables();
  eng.renderer->initAndClearPrev();

  eng.renderer->applySurface(Pos(0, 0), screenCpy, NULL);
  eng.renderer->updateScreen();

  vector<string> lines;
  collectLinesFromVariables(lines);
  writeLinesToFile(lines);
}

void Config::setAllVariablesFromLines(vector<string>& lines) {
  trace << "Config::setAllVariablesFromLines()..." << endl;

  string curLine = lines.front();
  isAudioEnabled = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  if(curLine == "0") {
    isTilesMode = false;
  } else {
    isTilesMode = true;
    if(cellW != 16 || cellH != 24) {
      fontBig = "images/16x24_v1.png";
      parseFontNameAndSetCellDims();
    }
  }
  lines.erase(lines.begin());

  curLine = lines.front();
  fontBig = curLine;
  parseFontNameAndSetCellDims();
  lines.erase(lines.begin());

  curLine = lines.front();
  isFullscreen = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isTilesWallSymbolFullSquare = curLine == "0" ? false : true;
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


