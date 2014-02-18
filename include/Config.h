#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

class Engine;
class MenuBrowser;

class Config {
public:
  Config(Engine& engine);
  ~Config() {}

  std::string   fontBig;

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
  int           cellW, cellH;

  void runOptionsMenu();

  void setCellDimDependentVariables();
  void toggleFullscreen();

private:
  void playerSetsOption(const MenuBrowser* const browser,
                        const int OPTION_VALUES_X_POS);

  void draw(const MenuBrowser* const browser, const int OPTION_VALUES_X_POS);

  void readFile(std::vector<std::string>& lines);
  void setAllVariablesFromLines(std::vector<std::string>& lines);
  void writeLinesToFile(std::vector<std::string>& lines);
  void setDefaultVariables();
  void collectLinesFromVariables(std::vector<std::string>& lines);
  void parseFontNameAndSetCellDims();

  std::vector<std::string> fontImageNames;

  Engine& eng;
};

#endif
