#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Engine;

namespace Config {

void init();
void runOptionsMenu(Engine& eng);
void setCellDimDependentVariables();
void toggleFullscreen(Engine& eng);

bool          isTilesMode();
std::string   getFontName();
bool          isFullscreen();
int           getScreenPixelW();
int           getScreenPixelH();
int           getCellW();
int           getCellH();
int           getLogPixelH();
int           getMapPixelH();
int           getMapPixelOffsetH();
int           getCharLinesPixelOffsetH();
int           getCharLinesPixelH();
bool          isAsciiWallSymbolFullSquare();
bool          isTilesWallSymbolFullSquare();
bool          isAudioEnabled();
bool          isBotPlaying();
void          setBotPlaying();
bool          isRangedWpnMeleeePrompt();
bool          isRangedWpnAutoReload();
bool          isIntroLevelSkipped();
int           getDelayProjectileDraw();
int           getDelayShotgun();
int           getDelayExplosion();
int           getKeyRepeatDelay();
int           getKeyRepeatInterval();

} //Config

#endif
