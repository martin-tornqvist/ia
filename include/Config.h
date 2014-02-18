#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Engine;

namespace Config {

extern std::string  fontBig;
extern bool         isFullscreen;
extern bool         isTilesWallSymbolFullSquare, isAsciiWallSymbolFullSquare;
extern bool         useRangedWpnMeleeePrompt;
extern bool         useRangedWpnAutoReload;
extern bool         isIntroLevelSkipped;
extern int          mapPixelH;
extern int          logPixelH;
extern int          mapPixelOffsetH;
extern int          charLinesPixelH;
extern int          charLinesPixelOffsetH;
extern int          screenPixelW, screenPixelH;
extern int          keyRepeatDelay, keyRepeatInterval;
extern int          delayProjectileDraw;
extern int          delayShotgun;
extern int          delayExplosion;
extern bool         isBotPlaying;
extern bool         isAudioEnabled;
extern bool         isTilesMode;
extern int          cellW, cellH;

void init();
void runOptionsMenu(Engine& eng);
void setCellDimDependentVariables();
void toggleFullscreen(Engine& eng);

} //Config

#endif
