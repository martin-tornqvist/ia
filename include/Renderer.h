#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <iostream>

#include <SDL_video.h>

#include "CommonData.h"
#include "GameTime.h"
#include "Config.h"
#include "Art.h"

struct Projectile;

enum class Panel {screen, map, charLines, log};

namespace Renderer {

extern CellRenderData renderArray[MAP_W][MAP_H];
extern CellRenderData renderArrayNoActors[MAP_W][MAP_H];
extern SDL_Surface*   screenSurface;
extern SDL_Surface*   mainMenuLogoSurface;

void init(Engine& engine);
void cleanup();

void drawMapAndInterface(const bool SHOULD_UPDATE_SCREEN = true);

void updateScreen();

void clearScreen();

void drawTile(const TileId tile, const Panel panel, const Pos& pos,
              const SDL_Color& clr, const SDL_Color& bgClr = clrBlack);

void drawGlyph(const char GLYPH, const Panel panel, const Pos& pos,
               const SDL_Color& clr, const bool DRAW_BG_CLR = true,
               const SDL_Color& bgClr = clrBlack);

void drawText(const string& str, const Panel panel, const Pos& pos,
              const SDL_Color& clr, const SDL_Color& bgClr = clrBlack);

int drawTextCentered(const string& str, const Panel panel, const Pos& pos,
                     const SDL_Color& clr, const SDL_Color& bgClr = clrBlack,
                     const bool IS_PIXEL_POS_ADJ_ALLOWED = true);

void coverCellInMap(const Pos& pos);

void coverPanel(const Panel panel);

void coverArea(const Panel panel, const Pos& pos, const Pos& dims);

void coverAreaPixel(const Pos& pixelPos, const Pos& pixelDims);

void drawRectangleSolid(const Pos& pixelPos, const Pos& pixelDims,
                        const SDL_Color& clr);

void drawLineHor(const Pos& pixelPos, const int W, const SDL_Color& clr);

void drawLineVer(const Pos& pixelPos, const int H, const SDL_Color& clr);

void drawMarker(const std::vector<Pos>& trail, const int EFFECTIVE_RANGE = -1);

void drawBlastAnimAtField(const Pos& centerPos, const int RADIUS,
                          bool forbiddenCells[MAP_W][MAP_H],
                          const SDL_Color& colorInner,
                          const SDL_Color& colorOuter);

void drawBlastAnimAtPositions(const std::vector<Pos>& positions,
                              const SDL_Color& color);

void drawBlastAnimAtPositionsWithPlayerVision(const vector<Pos>& positions,
    const SDL_Color& clr);

void drawMainMenuLogo(const int Y_POS);

void drawProjectiles(vector<Projectile*>& projectiles,
                     const bool SHOULD_DRAW_MAP_BEFORE);

void drawPopupBox(const Rect& area, const Panel panel = Panel::screen,
                  const SDL_Color& clr = clrGray);

void applySurface(const Pos& pixelPos, SDL_Surface* const src,
                  SDL_Rect* clip = NULL);

void drawMap();

} //Render

#endif

