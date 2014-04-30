#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace Config {

void init();
void runOptionsMenu();
void setCellDimDependentVariables();
void toggleFullscreen();

bool            isTilesMode();
std::string     getFontName();
bool            isFullscreen();
int             getScreenPixelW();
int             getScreenPixelH();
int             getCellW();
int             getCellH();
int             getLogPixelH();
int             getMapPixelH();
int             getMapPixelOffsetH();
int             getCharLinesPixelOffsetH();
int             getCharLinesPixelH();
bool            isAsciiWallFullSquare();
bool            isTilesWallFullSquare();
bool            isAudioEnabled();
bool            isBotPlaying();
void            setBotPlaying();
bool            isRangedWpnMeleeePrompt();
bool            isRangedWpnAutoReload();
bool            isIntroLevelSkipped();
int             getDelayProjectileDraw();
int             getDelayShotgun();
int             getDelayExplosion();
int             getKeyRepeatDelay();
int             getKeyRepeatInterval();

} //Config

#endif
