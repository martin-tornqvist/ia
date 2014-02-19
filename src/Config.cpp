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
#include "Utils.h"

using namespace std;

namespace Config {

//---------------------------------------------------------------- LOCAL
namespace {

const int OPT_Y0 = 2;

string  fontName_                     = "";
bool    isFullscreen_                 = false;
bool    isTilesWallSymbolFullSquare_  = false;
bool    isAsciiWallSymbolFullSquare_  = false;
bool    isRangedWpnMeleeePrompt_      = false;
bool    isRangedWpnAutoReload_        = false;
bool    isIntroLevelSkipped_          = false;
int     mapPixelH_                    = -1;
int     logPixelH_                    = -1;
int     mapPixelOffsetH_              = -1;
int     charLinesPixelH_              = -1;
int     charLinesPixelOffsetH_        = -1;
int     screenPixelW_                 = -1;
int     screenPixelH_                 = -1;
int     keyRepeatDelay_               = -1;
int     keyRepeatInterval_            = -1;
int     delayProjectileDraw_          = -1;
int     delayShotgun_                 = -1;
int     delayExplosion_               = -1;
bool    isBotPlaying_                 = false;
bool    isAudioEnabled_               = false;
bool    isTilesMode_                  = false;
int     cellW_                        = -1;
int     cellH_                        = -1;

vector<string> fontImageNames;

void parseFontNameAndSetCellDims() {
  trace << "Config::parseFontNameAndSetCellDims()..." << endl;
  string fontName = fontName_;

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

  cellW_ = toInt(wStr);
  cellH_ = toInt(hStr);
  trace << "Config::parseFontNameAndSetCellDims() [DONE]" << endl;
}

void setDefaultVariables() {
  trace << "Config::setDefaultVariables()..." << endl;
  isAudioEnabled_               = true;
  isTilesMode_                  = true;
  fontName_                      = "images/16x24_v1.png";
  parseFontNameAndSetCellDims();
  isFullscreen_ = false;
  isTilesWallSymbolFullSquare_  = false;
  isAsciiWallSymbolFullSquare_  = true;
  isIntroLevelSkipped_          = false;
  isRangedWpnMeleeePrompt_      = true;
  isRangedWpnAutoReload_        = false;
  keyRepeatDelay_               = 130;
  keyRepeatInterval_            = 60;
  delayProjectileDraw_          = 50;
  delayShotgun_                 = 120;
  delayExplosion_               = 350;
  trace << "Config::setDefaultVariables() [DONE]" << endl;
}

void playerSetsOption(const MenuBrowser* const browser,
                      const int OPTION_VALUES_X_POS,
                      Engine& eng) {
  switch(browser->getPos().y) {
    case 0: {
      isAudioEnabled_ = !isAudioEnabled_;
      eng.audio->initAndClearPrev();
    } break;

    case 1: {
      isTilesMode_ = !isTilesMode_;
      if(isTilesMode_ && (cellW_ != 16 || cellH_ != 24)) {
        fontName_ = "images/16x24_v1.png";
      }
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      Renderer::init(eng);
    } break;

    case 2: {
      for(unsigned int i = 0; i < fontImageNames.size(); i++) {
        if(fontName_ == fontImageNames.at(i)) {
          fontName_ = i == fontImageNames.size() - 1 ?
                     fontImageNames.front() :
                     fontImageNames.at(i + 1);
          break;
        }
      }
      parseFontNameAndSetCellDims();

      if(isTilesMode_) {
        while(cellW_ != 16 || cellH_ != 24) {
          for(unsigned int i = 0; i < fontImageNames.size(); i++) {
            if(fontName_ == fontImageNames.at(i)) {
              fontName_ = i == fontImageNames.size() - 1 ?
                         fontImageNames.front() :
                         fontImageNames.at(i + 1);
              break;
            }
          }
          parseFontNameAndSetCellDims();
        }
      }

      setCellDimDependentVariables();
      Renderer::init(eng);
    } break;

    case 3: {
      isFullscreen_ = !isFullscreen_;
      Renderer::init(eng);
    } break;

    case 4: {
      isTilesWallSymbolFullSquare_ = !isTilesWallSymbolFullSquare_;
    } break;

    case 5: {
      isAsciiWallSymbolFullSquare_ = !isAsciiWallSymbolFullSquare_;
    } break;

    case 6: {isIntroLevelSkipped_ = !isIntroLevelSkipped_;} break;

    case 7: {isRangedWpnMeleeePrompt_ = !isRangedWpnMeleeePrompt_;} break;

    case 8: {isRangedWpnAutoReload_ = !isRangedWpnAutoReload_;} break;

    case 9: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        eng.query->number(p, clrNosfTealLgt, 1, 3, keyRepeatDelay_, true);
      if(NR != -1) {
        keyRepeatDelay_ = NR;
        Input::setKeyRepeatDelays();
      }
    } break;

    case 10: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        eng.query->number(p, clrNosfTealLgt, 1, 3, keyRepeatInterval_, true);
      if(NR != -1) {
        keyRepeatInterval_ = NR;
        Input::setKeyRepeatDelays();
      }
    } break;

    case 11: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        eng.query->number(p, clrNosfTealLgt, 1, 3, delayProjectileDraw_, true);
      if(NR != -1) {delayProjectileDraw_ = NR;}
    } break;
    case 12: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        eng.query->number(p, clrNosfTealLgt, 1, 3, delayShotgun_, true);
      if(NR != -1) {
        delayShotgun_ = NR;
      }
    } break;

    case 13: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        eng.query->number(p, clrNosfTealLgt, 1, 3, delayExplosion_, true);
      if(NR != -1) {delayExplosion_ = NR;}
    } break;

    case 14: {
      setDefaultVariables();
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      Renderer::init(eng);
    } break;
  }
}

void draw(const MenuBrowser* const browser,
          const int OPTION_VALUES_X_POS) {

  const SDL_Color clrActive     = clrNosfTealLgt;
  const SDL_Color clrInactive   = clrNosfTealDrk;

  Renderer::clearScreen();

  int optNr = 0;

  const int X0 = 1;
  const int X1 = OPTION_VALUES_X_POS;

  string str = "";

  Renderer::drawText("-Options-", panel_screen, Pos(X0, 0), clrWhite);

  Renderer::drawText("Play audio", panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isAudioEnabled_ ? "Yes" : "No";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  Renderer::drawText("Use tile set", panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isTilesMode_ ? "Yes" : "No";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  Renderer::drawText("Font", panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  string fontDispName;
  TextFormatting::replaceAll(fontName_,      "images/",  "",   fontDispName);
  TextFormatting::replaceAll(fontDispName,  "_",        " ",  fontDispName);
  TextFormatting::replaceAll(fontDispName,  ".png",     "",   fontDispName);
  Renderer::drawText(fontDispName, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  Renderer::drawText("Fullscreen (experimental)", panel_screen,
                         Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(isFullscreen_ ? "Yes" : "No",
                         panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Tiles mode wall symbol";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isTilesWallSymbolFullSquare_ ? "Full square" : "Pseudo-3D";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Ascii mode wall symbol";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isAsciiWallSymbolFullSquare_ ? "Full square" : "Hash sign";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  Renderer::drawText("Skip intro level", panel_screen,
                         Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isIntroLevelSkipped_ ? "Yes" : "No";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Ranged weapon melee attack warning";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isRangedWpnMeleeePrompt_ ? "Yes" : "No";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Ranged weapon auto reload";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  str = isRangedWpnAutoReload_ ? "Yes" : "No";
  Renderer::drawText(str, panel_screen, Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Key repeat delay (ms)";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(toString(keyRepeatDelay_), panel_screen,
                         Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Key repeat interval (ms)";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(toString(keyRepeatInterval_), panel_screen,
                         Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Projectile delay (ms)";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(toString(delayProjectileDraw_), panel_screen,
                         Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Shotgun delay (ms)";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(toString(delayShotgun_), panel_screen,
                         Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Explosion delay (ms)";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(":", panel_screen, Pos(X1 - 2, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  Renderer::drawText(toString(delayExplosion_), panel_screen,
                         Pos(X1, OPT_Y0 + optNr),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);
  optNr++;

  str = "Reset to defaults";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr + 1),
                         browser->getPos().y == optNr ?
                         clrActive : clrInactive);

  str = "space/esc to confirm changes";
  Renderer::drawText(str, panel_screen, Pos(X0, OPT_Y0 + optNr + 4),
                         clrWhite);

  str  = "Tile set requires resolution 1280x720 or higher. ";
  str += "Using ASCII mode for smaller";
  Renderer::drawText(str, panel_screen, Pos(X0, SCREEN_H - 2), clrGray);
  str = "resolutions is recommended (fonts of different sizes are available).";
  Renderer::drawText(str, panel_screen, Pos(X0, SCREEN_H - 1), clrGray);

  Renderer::updateScreen();
}

void readFile(vector<string>& lines) {
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

void setAllVariablesFromLines(vector<string>& lines) {
  trace << "Config::setAllVariablesFromLines()..." << endl;

  string curLine = lines.front();
  isAudioEnabled_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  if(curLine == "0") {
    isTilesMode_ = false;
  } else {
    isTilesMode_ = true;
    if(cellW_ != 16 || cellH_ != 24) {
      fontName_ = "images/16x24_v1.png";
      parseFontNameAndSetCellDims();
    }
  }
  lines.erase(lines.begin());

  curLine = lines.front();
  fontName_ = curLine;
  parseFontNameAndSetCellDims();
  lines.erase(lines.begin());

  curLine = lines.front();
  isFullscreen_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isTilesWallSymbolFullSquare_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isAsciiWallSymbolFullSquare_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isIntroLevelSkipped_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isRangedWpnMeleeePrompt_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  isRangedWpnAutoReload_ = curLine == "0" ? false : true;
  lines.erase(lines.begin());

  curLine = lines.front();
  keyRepeatDelay_ = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  keyRepeatInterval_ = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  delayProjectileDraw_ = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  delayShotgun_ = toInt(curLine);
  lines.erase(lines.begin());

  curLine = lines.front();
  delayExplosion_ = toInt(curLine);
  lines.erase(lines.begin());
  trace << "Config::setAllVariablesFromLines() [DONE]" << endl;
}

void writeLinesToFile(vector<string>& lines) {
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

void collectLinesFromVariables(vector<string>& lines) {
  trace << "Config::collectLinesFromVariables()..." << endl;
  lines.resize(0);
  lines.push_back(isAudioEnabled_ == false ? "0" : "1");
  lines.push_back(isTilesMode_ == false ? "0" : "1");
  lines.push_back(fontName_);
  lines.push_back(isFullscreen_ == false ? "0" : "1");
  lines.push_back(isTilesWallSymbolFullSquare_ == false ? "0" : "1");
  lines.push_back(isAsciiWallSymbolFullSquare_ == false ? "0" : "1");
  lines.push_back(isIntroLevelSkipped_ == false ? "0" : "1");
  lines.push_back(isRangedWpnMeleeePrompt_ == false ? "0" : "1");
  lines.push_back(isRangedWpnAutoReload_ == false ? "0" : "1");
  lines.push_back(toString(keyRepeatDelay_));
  lines.push_back(toString(keyRepeatInterval_));
  lines.push_back(toString(delayProjectileDraw_));
  lines.push_back(toString(delayShotgun_));
  lines.push_back(toString(delayExplosion_));
  trace << "Config::collectLinesFromVariables() [DONE]" << endl;
}

} //Namespace

//---------------------------------------------------------------- GLOBAL
bool    isTilesMode()                 {return isTilesMode_;}
string  getFontName()                 {return fontName_;}
bool    isFullscreen()                {return isFullscreen_;}
int     getScreenPixelW()             {return screenPixelW_;}
int     getScreenPixelH()             {return screenPixelH_;}
int     getCellW()                    {return cellW_;}
int     getCellH()                    {return cellH_;}
int     getLogPixelH()                {return logPixelH_;}
int     getMapPixelH()                {return mapPixelH_;}
int     getMapPixelOffsetH()          {return mapPixelOffsetH_;}
int     getCharLinesPixelOffsetH()    {return charLinesPixelOffsetH_;}
int     getCharLinesPixelH()          {return charLinesPixelH_;}
bool    isAsciiWallSymbolFullSquare() {return isAsciiWallSymbolFullSquare_;}
bool    isTilesWallSymbolFullSquare() {return isTilesWallSymbolFullSquare_;}
bool    isAudioEnabled()              {return isAudioEnabled_;}
bool    isBotPlaying()                {return isBotPlaying_;}
void    setBotPlaying()               {isBotPlaying_ = true;}
bool    isRangedWpnMeleeePrompt()     {return isRangedWpnMeleeePrompt_;}
bool    isRangedWpnAutoReload()       {return isRangedWpnAutoReload_;}
bool    isIntroLevelSkipped()         {return isIntroLevelSkipped_;}
int     getDelayProjectileDraw()      {return delayProjectileDraw_;}
int     getDelayShotgun()             {return delayShotgun_;}
int     getDelayExplosion()           {return delayExplosion_;}
int     getKeyRepeatDelay()           {return keyRepeatDelay_;}
int     getKeyRepeatInterval()        {return keyRepeatInterval_;}

void init() {
  fontName_ = "";
  isBotPlaying_ = false;

  fontImageNames.resize(0);
  fontImageNames.push_back("images/8x12_DOS.png");
  fontImageNames.push_back("images/11x19.png");
  fontImageNames.push_back("images/11x22.png");
  fontImageNames.push_back("images/12x24.png");
  fontImageNames.push_back("images/16x24_v1.png");
  fontImageNames.push_back("images/16x24_v2.png");
  fontImageNames.push_back("images/16x24_v3.png");
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

void runOptionsMenu(Engine& eng) {
  MenuBrowser browser(15, 0);
  vector<string> lines;

  const int OPTION_VALUES_X_POS = 40;

  draw(&browser, OPTION_VALUES_X_POS);

  while(true) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        draw(&browser, OPTION_VALUES_X_POS);
      } break;

      case menuAction_esc:
      case menuAction_space: {
        //Since ASCII mode wall symbol may have changed,
        //we need to redefine the feature data list
        eng.featureDataHandler->initDataList();
        return;
      } break;

      case menuAction_selected: {
        draw(&browser, OPTION_VALUES_X_POS);
        playerSetsOption(&browser, OPTION_VALUES_X_POS, eng);
        collectLinesFromVariables(lines);
        writeLinesToFile(lines);
        draw(&browser, OPTION_VALUES_X_POS);
      } break;

      default: {} break;
    }
  }
}

void setCellDimDependentVariables() {
  mapPixelH_              = cellH_ * MAP_H;
  mapPixelOffsetH_        = cellH_ * MAP_OFFSET_H;
  logPixelH_              = cellH_ * LOG_H;
  charLinesPixelH_        = cellH_ * CHAR_LINES_H;
  charLinesPixelOffsetH_  = cellH_ * CHAR_LINES_OFFSET_H;
  screenPixelW_           = cellW_ * SCREEN_W;
  screenPixelH_           = cellH_ * SCREEN_H;
}

void toggleFullscreen(Engine& eng) {
  SDL_Surface* screenCpy = SDL_DisplayFormat(Renderer::screenSurface_);

  isFullscreen_ = !isFullscreen_;
  parseFontNameAndSetCellDims();
  setCellDimDependentVariables();
  Renderer::init(eng);

  Renderer::applySurface(Pos(0, 0), screenCpy, NULL);
  Renderer::updateScreen();

  vector<string> lines;
  collectLinesFromVariables(lines);
  writeLinesToFile(lines);
}

} //Config



