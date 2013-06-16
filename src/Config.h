#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#include "CommonTypes.h"
#include "CommonSettings.h"

using namespace std;

class Engine;
class MenuBrowser;

class Config {
public:
  Config(Engine* engine);

  ~Config() {
  }

  const string  GAME_VERSION;

  const string  TILES_IMAGE_NAME;
  const string  MAIN_MENU_LOGO_IMAGE_NAME;
  string        fontImageName;

  bool          isFullscreen;

  const int     LOG_X_CELLS_OFFSET;
  const int     LOG_Y_CELLS_OFFSET;
  const int     LOG_X_CELLS;
  bool          isAsciiWallSymbolFullSquare;
  bool          useRangedWpnMleeePrompt;
  bool          isIntroLevelSkipped;
  bool          isGunsAutoReloaded;
  int           mainscreenHeight;
  int           logHeight;
  const int     CHARACTER_LINES_Y_CELLS_OFFSET;
  const int     CHARACTER_LINES_Y_CELLS;
  int           CHARACTER_LINES_HEIGHT;
  int           screenWidth, screenHeight;
  const int     SCREEN_BPP;
  const int     FRAMES_PER_SECOND;
  const int     PLAYER_START_X;
  const int     PLAYER_START_Y;
  int           keyRepeatDelay, keyRepeatInterval;
  int           delayProjectileDraw;
  int           delayShotgun;
  int           delayExplosion;
  bool          isBotPlaying;
  bool          isTilesMode;
  int           logOffsetX, logOffsetY;
  const int     MAINSCREEN_Y_CELLS_OFFSET;
  int           mainscreenOffsetY;
  int           characterLinesOffsetY;
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

  Engine* const eng;
};

#endif
