#include "Config.h"

#include <fstream>
#include <iostream>

#include <SDL_image.h>

#include "Converters.h"
#include "MenuInputHandling.h"
#include "MenuBrowser.h"
#include "Query.h"
#include "Render.h"
#include "Input.h"
#include "Audio.h"
#include "TextFormatting.h"
#include "Utils.h"
#include "Init.h"


using namespace std;

namespace Config {

namespace {

const int NR_OPTIONS  = 15;
const int OPT_Y0      = 1;

string  fontName_                     = "";
bool    isFullscreen_                 = false;
bool    isTilesWallFullSquare_        = false;
bool    isAsciiWallFullSquare_        = false;
bool    isRangedWpnMeleeePrompt_      = false;
bool    isRangedWpnAutoReload_        = false;
bool    isIntroLvlSkipped_            = false;
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
  TRACE_FUNC_BEGIN;
  string fontName = fontName_;

  char ch = 'a';
  while(ch < '0' || ch > '9') {
    fontName.erase(begin(fontName));
    ch = fontName.at(0);
  }

  string wStr = "";
  while(ch != 'x') {
    fontName.erase(begin(fontName));
    wStr += ch;
    ch = fontName.at(0);
  }

  fontName.erase(begin(fontName));
  ch = fontName.at(0);

  string hStr = "";
  while(ch != '_' && ch != '.') {
    fontName.erase(begin(fontName));
    hStr += ch;
    ch = fontName.at(0);
  }

  TRACE << "Parsed font image name, found dims: "
        << wStr << "x" << hStr << endl;

  cellW_ = toInt(wStr);
  cellH_ = toInt(hStr);
  TRACE_FUNC_END;
}

void setDefaultVariables() {
  TRACE_FUNC_BEGIN;
  isAudioEnabled_               = true;
  isTilesMode_                  = true;
  fontName_                     = "images/16x24_v2.png";
  parseFontNameAndSetCellDims();
  isFullscreen_                 = false;
  isTilesWallFullSquare_        = false;
  isAsciiWallFullSquare_        = true;
  isIntroLvlSkipped_            = false;
  isRangedWpnMeleeePrompt_      = true;
  isRangedWpnAutoReload_        = false;
  keyRepeatDelay_               = 130;
  keyRepeatInterval_            = 60;
  delayProjectileDraw_          = 50;
  delayShotgun_                 = 120;
  delayExplosion_               = 350;
  TRACE_FUNC_END;
}

void playerSetsOption(const MenuBrowser* const browser,
                      const int OPTION_VALUES_X_POS) {
  switch(browser->getPos().y) {
    case 0: {
      isAudioEnabled_ = !isAudioEnabled_;
      Audio::init();
    } break;

    case 1: {
      isTilesMode_ = !isTilesMode_;
      if(isTilesMode_ && (cellW_ != 16 || cellH_ != 24)) {
        fontName_ = "images/16x24_v1.png";
      }
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      Render::init();
    } break;

    case 2: {
      for(unsigned int i = 0; i < fontImageNames.size(); ++i) {
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
          for(unsigned int i = 0; i < fontImageNames.size(); ++i) {
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
      Render::init();
    } break;

    case 3: {
      isFullscreen_ = !isFullscreen_;
      Render::init();
    } break;

    case 4: {
      isTilesWallFullSquare_ = !isTilesWallFullSquare_;
    } break;

    case 5: {
      isAsciiWallFullSquare_ = !isAsciiWallFullSquare_;
    } break;

    case 6: {isIntroLvlSkipped_ = !isIntroLvlSkipped_;} break;

    case 7: {isRangedWpnMeleeePrompt_ = !isRangedWpnMeleeePrompt_;} break;

    case 8: {isRangedWpnAutoReload_ = !isRangedWpnAutoReload_;} break;

    case 9: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        Query::number(p, clrMenuHighlight, 1, 3, keyRepeatDelay_, true);
      if(NR != -1) {
        keyRepeatDelay_ = NR;
        Input::setKeyRepeatDelays();
      }
    } break;

    case 10: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        Query::number(p, clrMenuHighlight, 1, 3, keyRepeatInterval_, true);
      if(NR != -1) {
        keyRepeatInterval_ = NR;
        Input::setKeyRepeatDelays();
      }
    } break;

    case 11: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        Query::number(p, clrMenuHighlight, 1, 3, delayProjectileDraw_, true);
      if(NR != -1) {delayProjectileDraw_ = NR;}
    } break;

    case 12: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        Query::number(p, clrMenuHighlight, 1, 3, delayShotgun_, true);
      if(NR != -1) {delayShotgun_ = NR;}
    } break;

    case 13: {
      const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->getPos().y);
      const int NR =
        Query::number(p, clrMenuHighlight, 1, 3, delayExplosion_, true);
      if(NR != -1) {delayExplosion_ = NR;}
    } break;

    case 14: {
      setDefaultVariables();
      parseFontNameAndSetCellDims();
      setCellDimDependentVariables();
      Render::init();
      Audio::init();
    } break;

    default: {assert(false && "Illegal option number");} break;
  }
}

void draw(const MenuBrowser* const browser, const int OPTION_VALUES_X_POS) {
  Render::clearScreen();

  int optNr = 0;

  const int X0 = 1;
  const int X1 = OPTION_VALUES_X_POS;

  string str = "";

  Render::drawText("-Options-", Panel::screen, Pos(X0, 0), clrWhite);

  Render::drawText("Play audio", Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isAudioEnabled_ ? "Yes" : "No";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  Render::drawText("Use tile set", Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isTilesMode_ ? "Yes" : "No";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  Render::drawText("Font", Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  string fontDispName;
  TextFormatting::replaceAll(fontName_,      "images/",  "",   fontDispName);
  TextFormatting::replaceAll(fontDispName,  "_",        " ",  fontDispName);
  TextFormatting::replaceAll(fontDispName,  ".png",     "",   fontDispName);
  Render::drawText(fontDispName, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  Render::drawText("Fullscreen (experimental)", Panel::screen,
                     Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(isFullscreen_ ? "Yes" : "No",
                     Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Tiles mode wall symbol";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isTilesWallFullSquare_ ? "Full square" : "Pseudo-3D";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Ascii mode wall symbol";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isAsciiWallFullSquare_ ? "Full square" : "Hash sign";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  Render::drawText("Skip intro level", Panel::screen,
                     Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isIntroLvlSkipped_ ? "Yes" : "No";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Ranged weapon melee attack warning";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isRangedWpnMeleeePrompt_ ? "Yes" : "No";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Ranged weapon auto reload";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  str = isRangedWpnAutoReload_ ? "Yes" : "No";
  Render::drawText(str, Panel::screen, Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Key repeat delay (ms)";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(toStr(keyRepeatDelay_), Panel::screen,
                     Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Key repeat interval (ms)";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(toStr(keyRepeatInterval_), Panel::screen,
                     Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Projectile delay (ms)";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(toStr(delayProjectileDraw_), Panel::screen,
                     Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Shotgun delay (ms)";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(toStr(delayShotgun_), Panel::screen,
                     Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Explosion delay (ms)";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  Render::drawText(toStr(delayExplosion_), Panel::screen,
                     Pos(X1, OPT_Y0 + optNr),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);
  optNr++;

  str = "Reset to defaults";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr + 1),
                     browser->getPos().y == optNr ?
                     clrMenuHighlight : clrMenuDrk);

  str = "[space/esc] to confirm changes";
  Render::drawText(str, Panel::screen, Pos(X0, OPT_Y0 + optNr + 4),
                     clrWhite);

  str  = "Tile set requires resolution 1280x720 or higher. ";
  str += "Using ASCII mode for smaller";
  Render::drawText(str, Panel::screen, Pos(X0, SCREEN_H - 2), clrGray);
  str = "resolutions is recommended (fonts of different sizes are available).";
  Render::drawText(str, Panel::screen, Pos(X0, SCREEN_H - 1), clrGray);

  Render::updateScreen();
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
  TRACE_FUNC_BEGIN;

  string curLine = lines.front();
  isAudioEnabled_ = curLine == "1";
  lines.erase(begin(lines));

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
  lines.erase(begin(lines));

  curLine = lines.front();
  fontName_ = curLine;
  parseFontNameAndSetCellDims();
  lines.erase(begin(lines));

  curLine = lines.front();
  isFullscreen_ = curLine == "1";
  lines.erase(begin(lines));

  curLine = lines.front();
  isTilesWallFullSquare_ = curLine == "1";
  lines.erase(begin(lines));

  curLine = lines.front();
  isAsciiWallFullSquare_ = curLine == "1";
  lines.erase(begin(lines));

  curLine = lines.front();
  isIntroLvlSkipped_ = curLine == "1";
  lines.erase(begin(lines));

  curLine = lines.front();
  isRangedWpnMeleeePrompt_ = curLine == "1";
  lines.erase(begin(lines));

  curLine = lines.front();
  isRangedWpnAutoReload_ = curLine == "1";
  lines.erase(begin(lines));

  curLine = lines.front();
  keyRepeatDelay_ = toInt(curLine);
  lines.erase(begin(lines));

  curLine = lines.front();
  keyRepeatInterval_ = toInt(curLine);
  lines.erase(begin(lines));

  curLine = lines.front();
  delayProjectileDraw_ = toInt(curLine);
  lines.erase(begin(lines));

  curLine = lines.front();
  delayShotgun_ = toInt(curLine);
  lines.erase(begin(lines));

  curLine = lines.front();
  delayExplosion_ = toInt(curLine);
  lines.erase(begin(lines));

  TRACE_FUNC_END;
}

void writeLinesToFile(vector<string>& lines) {
  ofstream file;
  file.open("config", ios::trunc);

  for(size_t i = 0; i < lines.size(); ++i) {
    file << lines.at(i);
    if(i != lines.size() - 1) {file << endl;}
  }

  file.close();
}

void collectLinesFromVariables(vector<string>& lines) {
  TRACE_FUNC_BEGIN;
  lines.clear();
  lines.push_back(isAudioEnabled_               ? "1" : "0");
  lines.push_back(isTilesMode_                  ? "1" : "0");
  lines.push_back(fontName_);
  lines.push_back(isFullscreen_                 ? "1" : "0");
  lines.push_back(isTilesWallFullSquare_        ? "1" : "0");
  lines.push_back(isAsciiWallFullSquare_        ? "1" : "0");
  lines.push_back(isIntroLvlSkipped_            ? "1" : "0");
  lines.push_back(isRangedWpnMeleeePrompt_      ? "1" : "0");
  lines.push_back(isRangedWpnAutoReload_        ? "1" : "0");
  lines.push_back(toStr(keyRepeatDelay_));
  lines.push_back(toStr(keyRepeatInterval_));
  lines.push_back(toStr(delayProjectileDraw_));
  lines.push_back(toStr(delayShotgun_));
  lines.push_back(toStr(delayExplosion_));
  TRACE_FUNC_END;
}

} //Namespace

void init() {
  fontName_ = "";
  isBotPlaying_ = false;

  fontImageNames.clear();
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
bool    isAsciiWallFullSquare()       {return isAsciiWallFullSquare_;}
bool    isTilesWallFullSquare()       {return isTilesWallFullSquare_;}
bool    isAudioEnabled()              {return isAudioEnabled_;}
bool    isBotPlaying()                {return isBotPlaying_;}
void    setBotPlaying()               {isBotPlaying_ = true;}
bool    isRangedWpnMeleeePrompt()     {return isRangedWpnMeleeePrompt_;}
bool    isRangedWpnAutoReload()       {return isRangedWpnAutoReload_;}
bool    isIntroLvlSkipped()         {return isIntroLvlSkipped_;}
int     getDelayProjectileDraw()      {return delayProjectileDraw_;}
int     getDelayShotgun()             {return delayShotgun_;}
int     getDelayExplosion()           {return delayExplosion_;}
int     getKeyRepeatDelay()           {return keyRepeatDelay_;}
int     getKeyRepeatInterval()        {return keyRepeatInterval_;}

void runOptionsMenu() {
  MenuBrowser browser(NR_OPTIONS, 0);
  vector<string> lines;

  const int OPTION_VALUES_X_POS = 40;

  draw(&browser, OPTION_VALUES_X_POS);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        draw(&browser, OPTION_VALUES_X_POS);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        //Since ASCII mode wall symbol may have changed,
        //we need to redefine the feature data list
        FeatureData::init();
        return;
      } break;

      case MenuAction::selected: {
        draw(&browser, OPTION_VALUES_X_POS);
        playerSetsOption(&browser, OPTION_VALUES_X_POS);
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

void toggleFullscreen() {
  SDL_Surface* screenCpy = SDL_DisplayFormat(Render::screenSurface);

  isFullscreen_ = !isFullscreen_;
  parseFontNameAndSetCellDims();
  setCellDimDependentVariables();
  Render::init();

  Render::applySurface(Pos(0, 0), screenCpy, nullptr);
  Render::updateScreen();

  vector<string> lines;
  collectLinesFromVariables(lines);
  writeLinesToFile(lines);
}

} //Config



