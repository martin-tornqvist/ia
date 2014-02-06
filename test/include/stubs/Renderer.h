#ifndef RENDERER_H
#define RENDERER_H

#include "Panel.h"

class Engine;
class Projectile;

class Renderer {
public:
  Renderer(Engine&) {}
  ~Renderer() {}

  void drawMapAndInterface(const bool = true) {}

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

  void drawLineHor(const Pos&, const int, const SDL_Color&) {}

  void drawLineVer(const Pos&, const int, const SDL_Color&) {}

  void drawMarker(const vector<Pos>&, const int = -1) {}

  void drawBlastAnimAtField(
    const Pos&, const int, bool [MAP_W][MAP_H],
    const SDL_Color&, const SDL_Color&) {}

  void drawBlastAnimAtPositions(const vector<Pos>&,
                                     const SDL_Color&) {}

  void drawBlastAnimAtPositionsWithPlayerVision(
    const vector<Pos>&, const SDL_Color&) {}

  void drawMainMenuLogo(const int) {}

  CellRenderData renderArray[MAP_W][MAP_H];
  CellRenderData renderArrayNoActors[MAP_W][MAP_H];

  void drawProjectiles(vector<Projectile*>&) {}

  void drawPopupBox(const Rect&, const Panel_t = panel_screen,
                    const SDL_Color& = clrGray) {}

  void applySurface(const Pos&, SDL_Surface* const, SDL_Rect* = NULL) {}

  void drawMap() {}

  SDL_Surface* screenSurface_;
};

#endif
