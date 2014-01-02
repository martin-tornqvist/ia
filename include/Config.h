#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#include "CommonData.h"

using namespace std;

class Engine;
class MenuBrowser;

class Config {
public:
  Config(Engine& engine);
  ~Config() {}

  string        fontBig;

  bool          isFullscreen;

  bool          isTilesWallSymbolFullSquare, isAsciiWallSymbolFullSquare;
  bool          useRangedWpnMeleeePrompt;
  bool          useRangedWpnAutoReload;
  bool          isIntroLevelSkipped;
  int           mapPixelH;
  int           logPixelH;
  int           mapPixelOffsetH;
  int           charLinesPixelH;
  int           charLinesPixelOffsetH;
  int           screenPixelW, screenPixelH;
  int           keyRepeatDelay, keyRepeatInterval;
  int           delayProjectileDraw;
  int           delayShotgun;
  int           delayExplosion;
  bool          isBotPlaying;
  bool          isAudioEnabled;
  bool          isTilesMode;
  int           fontScale;
  int           cellW, cellH;

  void runOptionsMenu();

  void setCellDimDependentVariables();
  void toggleFullscreen();

private:
  void playerSetsOption(const MenuBrowser* const browser,
                        const int OPTION_VALUES_X_POS,
                        const int OPTIONS_Y_POS);

  void draw(const MenuBrowser* const browser, const int OPTION_VALUES_X_POS,
            const int OPTIONS_Y_POS);

  void readFile(vector<string>& lines);
  void setAllVariablesFromLines(vector<string>& lines);
  void writeLinesToFile(vector<string>& lines);
  void setDefaultVariables();
  void collectLinesFromVariables(vector<string>& lines);
  void parseFontNameAndSetCellDims();

  vector<string> fontImageNames;

  Engine& eng;
};

#endif
