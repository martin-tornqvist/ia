#ifndef RENDERER_H
#define RENDERER_H

#include "Panel.h"

class Engine;
class Projectile;

class Renderer {
public:
  Renderer(Engine&) {}
  ~Renderer() {}

  void drawMapAndInterface(const bool = false) {}

  inline void updateScreen() {}

  inline void clearScreen() {}

  void initAndClearPrev() {}

  void drawTile(const Tile_t, const Panel_t, const Pos&, const SDL_Color&,
                const SDL_Color& = clrBlack) {}

  void drawGlyph(const char, const Panel_t, const Pos&,
                 const SDL_Color&, const bool = true,
                 const SDL_Color& = clrBlack) {}

  void drawText(const string&, const Panel_t, const Pos&, const SDL_Color&,
                const SDL_Color& = clrBlack) {}

  int drawTextCentered(const string&, const Panel_t, const Pos&,
                       const SDL_Color&, const SDL_Color& = clrBlack,
                       const bool = true) {return 0;}

  void coverCellInMap(const Pos&) {}

  void coverPanel(const Panel_t) {}

  void coverArea(const Panel_t, const Pos&, const Pos&) {}

  void coverAreaPixel(const Pos&, const Pos&) {}

  void drawRectangleSolid(const Pos&, const Pos&, const SDL_Color&) {}

  void drawLineHorizontal(const Pos&, const int, const SDL_Color&) {}

  void drawLineVertical(const Pos&, const int, const SDL_Color&) {}

  void drawMarker(const vector<Pos>&, const int = -1) {}

  void drawBlastAnimationAtField(
    const Pos&, const int, bool [MAP_X_CELLS][MAP_Y_CELLS],
    const SDL_Color&, const SDL_Color&) {}

  void drawBlastAnimationAtPositions(const vector<Pos>&,
                                     const SDL_Color&) {}

  void drawBlastAnimationAtPositionsWithPlayerVision(
    const vector<Pos>&, const SDL_Color&) {}

  void drawMainMenuLogo(const int) {}

  CellRenderDataAscii renderArrayActorsOmittedAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles renderArrayActorsOmittedTiles[MAP_X_CELLS][MAP_Y_CELLS];

  CellRenderDataAscii renderArrayAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles renderArrayTiles[MAP_X_CELLS][MAP_Y_CELLS];

  void drawProjectiles(vector<Projectile*>&) {}

  void drawPopupBox(const Rect&, const Panel_t,
                    const SDL_Color& = clrGray) {}

  void applySurface(const Pos&, SDL_Surface* const, SDL_Rect* = NULL) {}

  void drawAscii() {}

  SDL_Surface* screenSurface_;
};

#endif
